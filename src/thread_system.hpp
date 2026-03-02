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

        }

        void batchPush(
            std::vector<std::function<void()>>& tasks,
            TaskPriority priority = TaskPriority::Normal,
            const std::string& description = ""
        ) {

        }

        bool pop(std::function<void()>& task) {

        }

        void stop() {

        }

        bool isEmpty() const {

        }

        bool isStopping() const {

        }

        void reserve(size_t capacity) {

        }

        size_t capacity() const {

        }

        size_t size() const {

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

        }

        size_t getTotalTasksProcessed() const {

        }

        size_t getTotalTasksEnqueued() const {

        }

        int64_t getTimeSinceLastEnqueue() const {

        }

        void notifyAllThreads() {}

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

    class Simulacrum_API ThreadSystem {
    public:
        static ThreadSystem& Instance() {
            static ThreadSystem instance;
            return instance;
        }

    private:
        ThreadSystem(const ThreadSystem&) = delete;
        ThreadSystem& operator=(const ThreadSystem&) = delete;

        ThreadSystem() = default;
    };

} // namespace Simulacrum
