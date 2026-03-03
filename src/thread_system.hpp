#pragma once
#include "macros.hpp"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>

namespace Simulacrum {

    /// @brief Task execution priority
    /// ```md
    /// - Critical: Must execute immediately (e.g. rendering, input)
    /// - High: Important tasks (e.g. physics, animation)
    /// - Normal: Default priority for most tasks
    /// - Low: Background tasks (e.g. asset loading)
    /// - Idle: Only execute when nothing else is pending
    /// ```
    enum class TaskPriority : uint8_t {
        Critical = 0,
        High = 1,
        Normal = 2,
        Low = 3,
        Idle = 4
    };

    /// @brief Task wrapper with priority information
    struct PrioritizedTask {
        std::function<void()> task;
        TaskPriority priority;
        std::chrono::steady_clock::time_point enqueue_time;
        std::string description;

        PrioritizedTask()
            : priority(TaskPriority::Normal)
            , enqueue_time(std::chrono::steady_clock::now())
        {}

        PrioritizedTask(
            std::function<void()> task,
            TaskPriority priority,
            std::string description
        )
            : task(std::move(task))
            , priority(priority)
            , enqueue_time(std::chrono::steady_clock::now())
            , description(std::move(description))
        {}

        bool operator<(const PrioritizedTask &other) const {
            // Higher priority (lower enum value) comes first
            if (priority != other.priority) {
                return priority > other.priority;
            }

            // If same priority, older tasks come first (FIFO)
            return enqueue_time > other.enqueue_time;
        }
    };

    struct WorkerBudget;

    /// @brief Thread-safe prioritized task queue using separate queues per priority.
    ///
    /// This class provides a thread-safe queue for tasks to be executed by
    /// the worker threads. Uses separate queues for each priority level to
    /// reduce lock contention and improve performance.
    ///
    /// The queues automatically grow as needed, but can also have capacity
    /// reserved in advance for better performance when submitted large
    /// numbers of tasks at once.
    class TaskQueue {
    public:
        /// @brief Construct a new TaskQueue
        /// @param initial_capacity Initial capacity to reserve per priority (default: 256)
        /// @param profile Enable detailed task profiling (default: false)
        explicit TaskQueue(size_t initial_capacity = 256, bool profile = false) {
            for (int i = 0; i <= static_cast<int>(TaskPriority::Idle); ++i) {

            }
        }

        void push(
            std::function<void()> task,
            TaskPriority priority = TaskPriority::Normal,
            const std::string &description = ""
        ) {
            int const priority_index = static_cast<int>(priority);

            last_enqueue_time.store(
                std::chrono::steady_clock::now(),
                std::memory_order_relaxed
            );

            {
                std::unique_lock<std::mutex> lock(priority_mutexes_[priority_index]);

                // Add the new task (deque handles capacity automatically)
                priority_queues_[priority_index].emplace_back(
                    std::move(task),
                    priority,
                    description
                );

                // Update the atomic counter
                priority_counts_[priority_index].count.fetch_add(1, std::memory_order_relaxed);

                // Set bitmask bit to indicate this queue has tasks
                queue_bitmask_.fetch_or(1 << priority_index, std::memory_order_relaxed);

                // Update statistics
                task_stats_[priority_index].enqueued++;
                total_tasks_enqueued_.fetch_add(1, std::memory_order_relaxed);

                if (priority <= TaskPriority::High && !description.empty()) {
                    spdlog::info(
                        "High priority task enqueued: {} (Priority: {})",
                        description,
                        priority_index
                    );
                }
            }

            // Smart notification: notify all for critical, otherwise notify one
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (priority == TaskPriority::Critical) {
                // Wake all for critical tasks to ensure immediate pickup
                condition_.notify_all();
            } else {
                // Wake one for all other tasks to prevent a thundering herd
                condition_.notify_one();
            }
        }

        /// @brief Batch enqueue multiple tasks with a single lock acquistion
        ///
        /// This method is highly optimized for scenarios where many tasks need to be
        /// submitted at once (e.g. entity updates, particle batches). It reduces
        /// lock contention from O(N) to O(1) by acquiring the mutex only once.
        ///
        /// @param tasks Vector of tasks to enqueue (will be moved from)
        /// @param priority Priority level for all tasks in the batch
        /// @param description Optional description prefix for debugging
        void batchPush(
            std::vector<std::function<void()>>& tasks,
            TaskPriority priority = TaskPriority::Normal,
            const std::string& description = ""
        ) {
            if (tasks.empty()) {
                return;
            }

            last_enqueue_time.store(
                std::chrono::steady_clock::now(),
                std::memory_order_relaxed
            );

            int priority_index = static_cast<int>(priority);
            size_t batch_size = tasks.size();

            {
                std::unique_lock<std::mutex> lock(priority_mutexes_[priority_index]);

                // Reserve space to avoid reallocations (deque doesn't have reserve, but
                // this is good practice)
                for (auto& task : tasks) {
                    priority_queues_[priority_index].emplace_back(
                        std::move(task),
                        priority,
                        description
                    );
                }

                // Update atomic counter once for entire batch
                priority_counts_[priority_index].count.fetch_add(batch_size, std::memory_order_relaxed);

                // Set bitmask bit to indicate this queue has tasks
                queue_bitmask_.fetch_or(1 << priority_index, std::memory_order_relaxed);

                // Update statistics
                task_stats_[priority_index].enqueued += batch_size;
                total_tasks_enqueued_.fetch_add(batch_size, std::memory_order_relaxed);

                // Log batch submissions
                if (!description.empty()) {
                    spdlog::info(
                        "Batch enqueued {} tasks: {} (Priority: {})",
                        batch_size,
                        description,
                        priority_index
                    );
                }
            }

            // Efficient wake strategy: minimize thundering herd while ensuring work gets
            // picked up
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (priority == TaskPriority::Critical) {
                condition_.notify_all();
            } else if (batch_size >= 16) {
                condition_.notify_all();
            } else {
                condition_.notify_one();
            }
        }

        bool pop(std::function<void()>& task) {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            // Wait indefinitely for tasks - notify_one/notify_all will wake us instantly
            // when tasks arrive
            condition_.wait(lock, [this] {
                return stopping_.load(std::memory_order_acquire) || hasAnyTasksLockFree();
            });

            if (stopping_.load(std::memory_order_acquire)) {
                return false;
            }

            lock.unlock();
            return tryPopTask(task);
        }

        void stop() {
            stopping_.store(true, std::memory_order_release);
            notifyAllThreads();

            // Clear queues WITHOUT holding queue mutex to avoid deadlock
            // Workers need queue mutex to check stopping flag during condition_.wait()
            for (int i = 0; i <= static_cast<int>(TaskPriority::Idle); ++i) {
                std::lock_guard<std::mutex> priority_lock(priority_mutexes_[i]);
                priority_queues_[i].clear();
                priority_counts_[i].count.store(0, std::memory_order_relaxed);
            }

            // Clear all bitmasks
            queue_bitmask_.store(0, std::memory_order_relaxed);

            // Wake again after clearing to ensure workers see empty queues
            notifyAllThreads();
        }

        bool isEmpty() const {
            // Use atomic counters for lock-free checking
            constexpr int max_priority = static_cast<int>(TaskPriority::Idle);
            return !std::any_of(
                priority_counts_.begin(),
                priority_counts_.begin() + max_priority + 1,
                [](const auto& counter) {
                    return counter.count.load(std::memory_order_relaxed) > 0;
                }
            );
        }

        bool isStopping() const {
            return stopping_.load(std::memory_order_acquire);
        }

        void reserve(size_t capacity) {
            if (capacity <= desired_capacity_) {
                return;
            }

            // std::deque doesn't have reserve() but we track desired capacity
            desired_capacity_ = capacity;
        }

        size_t capacity() const {
            return desired_capacity_;
        }

        size_t size() const {
            constexpr int max_priority = static_cast<int>(TaskPriority::Idle);
            return std::accumulate(
                priority_counts_.begin(),
                priority_counts_.begin() + max_priority + 1,
                size_t{0},
                [](size_t sum, const auto& counter) {
                    return sum + counter.count.load(std::memory_order_relaxed);
                }
            );
        }

        struct TaskStats {
            size_t enqueued{0};
            size_t completed{0};
            size_t total_wait_time_ms{0};

            double getAverageWaitTime() const {
                return completed > 0 ? static_cast<double>(total_wait_time_ms) / completed : 0.0;
            }
        };

        TaskStats getTaskStats(TaskPriority priority) const {
            int const index = static_cast<int>(priority);
            if (index >= 0 && index <= static_cast<int>(TaskPriority::Idle)) {
                return task_stats_[index];
            }

            return TaskStats{};
        }

        size_t getTotalTasksProcessed() const {
            return total_tasks_processed_.load(std::memory_order_relaxed);
        }

        size_t getTotalTasksEnqueued() const {
            return total_tasks_enqueued_.load(std::memory_order_relaxed);
        }

        bool hasTasks() {
            return hasAnyTasksLockFree();
        }

        int64_t getTimeSinceLastEnqueue() const {
            auto now = std::chrono::steady_clock::now();
            auto last_enqueue = last_enqueue_time.load(std::memory_order_relaxed);
            return std::chrono::duration_cast<std::chrono::milliseconds>(now - last_enqueue).count();
        }

        void notifyAllThreads() {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            condition_.notify_all();
        }

        std::condition_variable& getCondition() { return condition_; }
        std::mutex& getMutex() { return queue_mutex_; }

    private:
        /// @brief Separate dequeues for each priority level
        /// (O(1) pop_front, reduces lock contention)
        mutable std::array<std::deque<PrioritizedTask>, 5> priority_queues_{};

        /// @brief Cache-line aligned mutexes to prevent false sharing
        alignas(64) mutable std::array<std::mutex, 5> priority_mutexes_;

        /// @brief Cache-line aligned atomic counts to prevent false sharing
        struct AlignedAtomic {
            std::atomic<size_t> count{0};
        };
        alignas(64) mutable std::array<AlignedAtomic, 5> priority_counts_{};

        /// @brief Bitmask tracking non-empty queues for fast skip in tryPopTask
        std::atomic<uint8_t> queue_bitmask_{0};

        /// @brief Main mutex for condition variable
        mutable std::mutex queue_mutex_{};
        std::condition_variable condition_{};
        std::atomic<bool> stopping_{false};

        /// @brief Statistics tracking with cache-friendly array instead of map
        mutable std::array<TaskStats, 5> task_stats_{};
        std::atomic<size_t> total_tasks_processed_{0};
        std::atomic<size_t> total_tasks_enqueued_{0};

        size_t desired_capacity_{256};
        bool enable_profiling_{false};

        /// @brief Track last time a task was enqueued for low-activity detection
        std::atomic<std::chrono::steady_clock::time_point> last_enqueue_time{std::chrono::steady_clock::now()};

        /// @brief Lock-free check for any tasks using bitmask (O(1) single atomic load)
        bool hasAnyTasksLockFree() const {
            return queue_bitmask_.load(std::memory_order_relaxed) != 0;
        }

        /// @brief Try to pop a task without blocking
        bool tryPopTask(std::function<void()> &task) {
            // Fast-path: Check bitmask to skip empty queues
            uint8_t bitmask = queue_bitmask_.load(std::memory_order_relaxed);

            // Try to get task from highest priority queues first
            for (
                int priority_index = 0;
                priority_index <= static_cast<int>(TaskPriority::Idle);
                ++priority_index
            ) {
                // Skip this priority level if bitmask indicates it's empty
                if (!bitmask & (1 << priority_index)) {
                    continue;
                }

                std::unique_lock<std::mutex> priority_lock(
                    priority_mutexes_[priority_index],
                    std::try_to_lock
                );

                if (!priority_lock.owns_lock()) {
                    // Skip if we can't get the lock immediately
                    continue;
                }

                auto& queue = priority_queues_[priority_index];

                if (!queue.empty()) {
                    // Get the oldest task from this priority level (FIFO within priority)
                    PrioritizedTask prioritized_task = std::move(queue.front());
                    queue.pop_front();

                    // Update atomic counter
                    size_t new_count = priority_counts_[priority_index].count.fetch_sub(1, std::memory_order_relaxed) - 1;

                    // Clear bitmask bit if queue is now empty
                    if (new_count == 0) {
                        queue_bitmask_.fetch_and(~(1 << priority_index), std::memory_order_relaxed);
                    }

                    // Calculate wait time for metrics
                    auto now = std::chrono::steady_clock::now();
                    auto wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - prioritized_task.enqueue_time
                    ).count();

                    // Update statistics if profiling is enabled
                    task_stats_[priority_index].completed++;
                    task_stats_[priority_index].total_wait_time_ms += wait_time;

                    // Log long wait times for high priority tasks
                    if (
                        priority_index <= static_cast<int>(TaskPriority::High) &&
                        wait_time > 100 &&
                        !prioritized_task.description.empty()
                    ) {
                        spdlog::warn(
                            "High priority task delayed: {} waited {}ms",
                            prioritized_task.description,
                            wait_time
                        );
                    }

                    // Return the actual task
                    task = std::move(prioritized_task.task);
                    total_tasks_processed_.fetch_add(1, std::memory_order_relaxed);
                    return true;
                }
            }

            return false;
        }
    };

    class ThreadPool {
    public:
        explicit ThreadPool(size_t num_threads, size_t queue_capacity = 256)
            : task_queue_(queue_capacity)
        {
            workers_.reserve(num_threads);

            for (size_t i = 0; i < num_threads; ++i) {
                workers_.emplace_back([this, i] {
                    workerThread(i);
                });
            }
        }

        ~ThreadPool() {
            is_running_.store(false, std::memory_order_relaxed);
            task_queue_.stop();

            for (auto& worker : workers_) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }

        void enqueue(
            std::function<void()> task,
            TaskPriority priority = TaskPriority::Normal,
            const std::string& description = ""
        ) {
            task_queue_.push(std::move(task), priority, description);
            total_tasks_enqueued_.fetch_add(1, std::memory_order_relaxed);
        }

        void batch_enqueue(
            std::vector<std::function<void()>>& tasks,
            TaskPriority priority = TaskPriority::Normal,
            const std::string& description = ""
        ) {
            if (tasks.empty()) {
                return;
            }

            size_t batch_size = tasks.size();
            task_queue_.batchPush(tasks, priority, description);

            total_tasks_enqueued_.fetch_add(batch_size, std::memory_order_relaxed);
        }

        bool busy() const {
            if (!task_queue_.isEmpty()) {
                return true;
            }

            return active_tasks_.load(std::memory_order_relaxed) > 0;
        }

        TaskQueue& getTaskQueue() { return task_queue_; }

        size_t getTotalTasksEnqueued() const {
            return total_tasks_enqueued_.load(std::memory_order_relaxed);
        }

        size_t getTotalTasksProcessed() const {
            return total_tasks_processed_.load(std::memory_order_relaxed);
        }

        template<class F, class... Args>
        auto enqueueWithResult(
            F&& f,
            TaskPriority priority = TaskPriority::Normal,
            const std::string& description = "",
            Args&& ...args
        ) -> std::future<typename std::invoke_result<F, Args...>::type> {
            using return_type = typename std::invoke_result<F, Args...>::type;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

            std::future<return_type> result = task->get_future();
            enqueue([task]() { (*task)(); }, priority, description);

            return result;
        }

    private:
        std::vector<std::thread> workers_;
        TaskQueue task_queue_;
        std::atomic<bool> is_running_{true};
        mutable std::atomic<size_t> active_tasks_{0};
        mutable std::mutex mutex_{};

        std::atomic<size_t> total_tasks_enqueued_{0};
        std::atomic<size_t> total_tasks_processed_{0};

        void workerThread(size_t thread_index = 0) {
            std::function<void()> task;

            auto start_time = std::chrono::steady_clock::now();
            size_t tasks_processed = 0;
            size_t high_priority_tasks = 0;

            try {
                auto last_task_time = std::chrono::steady_clock::now();
                std::chrono::steady_clock::time_point idle_start_time;
                bool is_idle = false;

                constexpr int64_t MIN_IDLE_TIME_MS = 20000;
                constexpr int64_t MIN_IDLE_EXIT_LOG_MS = 100;

                // Main worker loop
                while (is_running_.load(std::memory_order_acquire)) {
                    // Check for shutdown immediately at loop start
                    if (!is_running_.load(std::memory_order_acquire)) {
                        break;
                    }

                    // Reset got_task at the start of each iteration
                    bool got_task = false;

                    try {

                        if (task_queue_.pop(task)) {
                            got_task = true;
                            high_priority_tasks++;
                            last_task_time = std::chrono::steady_clock::now();
                        }

                    }

                    catch (...) {
                        // If any exception occurs during pop, check shutdown
                        if (!is_running_.load(std::memory_order_acquire)) {
                            break;
                        }
                        continue;
                    }

                    // Check shutdown again after getting task
                    if (!is_running_.load(std::memory_order_acquire)) {
                        break;
                    }

                    if (got_task) {

                        // Exiting idle mode - log if we were previously idle for a
                        // meaningful duration
                        if (is_idle) {
                            auto idle_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - idle_start_time
                            ).count();

                            if (idle_time >= MIN_IDLE_EXIT_LOG_MS) {
                                spdlog::info(
                                    "Worker {} exiting idle mode (was idle for {}ms)",
                                    thread_index,
                                    idle_time
                                );
                            }

                            is_idle = false;
                        }

                        const size_t active_count = active_tasks_.fetch_add(1, std::memory_order_acquire) + 1;

                        auto task_start_time = std::chrono::steady_clock::now();

                        try {
                            // Execute the task and increment counter
                            task();
                            tasks_processed++;
                            total_tasks_processed_.fetch_add(1, std::memory_order_acquire);
                        }

                        catch (const std::exception& exc) {
                            spdlog::error(
                                "Error in worker thread {}: {}",
                                thread_index,
                                exc.what()
                            );
                        }

                        catch (...) {
                            spdlog::error("Unknown error in worker thread {}", thread_index);
                        }

                        // Decrement with relaxe ordering
                        active_tasks_.fetch_sub(1, std::memory_order_relaxed);

                        auto task_end_time = std::chrono::steady_clock::now();
                        auto task_duration =
                            std::chrono::duration_cast<std::chrono::milliseconds>(
                                task_end_time - task_start_time
                            ).count();

                        if (task_duration > 100) {
                            spdlog::warn(
                                "Worker {} - Slow task: {}ms{}",
                                thread_index,
                                task_duration,
                                (high_priority_tasks > 0 ? " (HIGH PRIORITY) " : "")
                            );
                        }

                        // Clear task after execution to free resources
                        task = nullptr;

                        (void)active_count;
                    }

                    else {
                        // No task available - only mark as idle and log if we've been
                        // without tasks long enough
                        if (!is_idle) {
                            auto time_since_last_task = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - last_task_time
                            ).count();

                            if (time_since_last_task >= MIN_IDLE_TIME_MS) {
                                spdlog::info(
                                    "Worker {} entering idle mode (no tasks for {}ms)",
                                    thread_index,
                                    time_since_last_task
                                );

                                is_idle = true;
                                idle_start_time = std::chrono::steady_clock::now();
                            }
                        }

                        // Worker will loop back and block in pop() until a task arrives
                    }
                }
            }

            catch (const std::exception& exc) {
                spdlog::error(
                    "Worker thread {} terminated with exception: {}",
                    thread_index,
                    exc.what()
                );
            }

            catch (...) {
                spdlog::error("Worker thread {} terminated with unknown exception", thread_index);
            }

            auto end_time = std::chrono::steady_clock::now();
            auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time
            ).count();

            spdlog::info(
                "Worker {} exiting after processing {} tasks over {}ms",
                thread_index,
                tasks_processed,
                total_duration
            );

            (void)tasks_processed;
            (void)total_duration;
            (void)high_priority_tasks;
        }
    };

    class ThreadSystem {
    public:
        // Task queue settings
        static constexpr size_t DEFAULT_QUEUE_CAPACITY = 4096;

        // Timeout settings
        static constexpr int DEFAULT_SHUTDOWN_TIMEOUT_MS = 5000;
        static constexpr int DEFAULT_TASK_TIMEOUT_MS = 30000;

        static ThreadSystem& Instance() {
            static ThreadSystem instance;
            return instance;
        }

        static bool Exists() {
            return !Instance().is_shutdown_.load(std::memory_order_acquire);
        }

        void clean() {
            // Set shutdown flag first so any new accesses will be rejected
            is_shutdown_.store(true, std::memory_order_release);

            // Ensure visibility across all threads
            std::atomic_thread_fence(std::memory_order_seq_cst);

            if (thread_pool_) {
                // First signal the pool to stop accepting new tasks
                // We don't need to wait for pending tasks to complete here
                thread_pool_->getTaskQueue().notifyAllThreads();

                // Allow a very brief delay for threads to notice shutdown signal
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

                thread_pool_.reset();

                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }

        ~ThreadSystem() {
            if (!is_shutdown_) {
                clean();
            }
        }

        bool init(
            size_t queue_capacity = DEFAULT_QUEUE_CAPACITY,
            unsigned int custom_thread_count = 0
        ) {
            if (is_shutdown_.load(std::memory_order_acquire)) {
                return false;
            }

            queue_capacity_ = queue_capacity;

            if (custom_thread_count > 0) {
                num_threads_ = custom_thread_count;
            } else {
                unsigned int hardware_threads = std::thread::hardware_concurrency();
                num_threads_ = (hardware_threads > 1) ? (hardware_threads - 1) : 1;
            }

            try {
                thread_pool_ = std::make_unique<ThreadPool>(num_threads_, queue_capacity_);
                spdlog::info("ThreadSystem initialized with {} worker threads", num_threads_);
                return thread_pool_ != nullptr;
            } catch (const std::exception& exc) {
                spdlog::error("Failed to initialize ThreadSystem: {}", exc.what());
                return false;
            }
        }

        void enqueueTask(
            std::function<void()> task,
            TaskPriority priority = TaskPriority::Normal,
            const std::string& description = ""
        ) {
            if (is_shutdown_.load(std::memory_order_acquire) || !thread_pool_) {
                return;
            }

            thread_pool_->enqueue(std::move(task), priority, description);
        }

        void batchEnqueueTasks(
            std::vector<std::function<void()>>& tasks,
            TaskPriority priority = TaskPriority::Normal,
            const std::string& description = ""
        ) {
            if (is_shutdown_.load(std::memory_order_acquire) || !thread_pool_) {
                return;
            }

            if (tasks.empty()) {
                return;
            }

            thread_pool_->batch_enqueue(tasks, priority, description);
        }

        template <class F, class... Args>
        auto
        enqueueTaskWithResult(
            F &&f,
            TaskPriority priority = TaskPriority::Normal,
            const std::string &description = "",
            Args&& ...args
        ) -> std::future<typename std::invoke_result<F, Args...>::type> {
            if (is_shutdown_.load(std::memory_order_acquire) || !thread_pool_) {
                using ResultType = typename std::invoke_result<F, Args...>::type;
                std::promise<ResultType> promise;

                try {
                    if constexpr (std::is_void_v<ResultType>) {
                        promise.set_value();
                    } else if constexpr (std::is_default_constructible_v<ResultType>) {
                        promise.set_value(ResultType{});
                    } else if constexpr (std::is_pointer_v<ResultType>) {
                        promise.set_value(nullptr);
                    } else {
                        promise.set_exception(std::make_exception_ptr(std::runtime_error(
                            "ThreadSystem shutdown: Cannot create default value"
                        )));
                    }
                } catch (...) {
                    promise.set_exception(std::current_exception());
                }

                return promise.get_future();
            }

            try {
                return thread_pool_->enqueueWithResult(
                    std::forward<F>(f),
                    priority,
                    description,
                    std::forward<Args>(args)...
                );
            } catch (const std::exception& exc) {
                spdlog::error("Error enqueueing task: {}", exc.what());
                throw;
            }
        }

        bool isBusy() const {
            if (is_shutdown_.load(std::memory_order_acquire) || !thread_pool_) {
                return false;
            }

            std::lock_guard<std::mutex> lock(mutex_);
            return thread_pool_->busy();
        }

        unsigned int getThreadCount() const { return num_threads_; }

        bool isShutdown() const { return is_shutdown_.load(std::memory_order_acquire); }

        size_t getQueueCapacity() const {
            if (thread_pool_) {
                return thread_pool_->getTaskQueue().capacity();
            }
            return queue_capacity_;
        }

        size_t getQueueSize() const {
            if (thread_pool_) {
                return thread_pool_->getTaskQueue().size();
            }

            return 0;
        }

    private:
        std::unique_ptr<ThreadPool> thread_pool_{nullptr};
        unsigned int num_threads_{};
        size_t queue_capacity_{DEFAULT_QUEUE_CAPACITY};
        std::atomic<bool> is_shutdown_{false};
        mutable std::mutex mutex_{};

        ThreadSystem(const ThreadSystem&) = delete;
        ThreadSystem& operator=(const ThreadSystem&) = delete;

        ThreadSystem() = default;
    };

} // namespace Simulacrum
