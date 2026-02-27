#pragma once

#include <string>
#include <vector>

namespace engine::utils {

    /// @brief Resolves resource paths across different execution contexts.
    ///
    /// Handles path resolution for:
    /// - Direct execution from project directory
    /// - Future modding support via search path priorities
    ///
    /// THREAD-SAFETY NOTE
    /// ResourcePath::init() must be called exactly once during single-threaded
    /// initialization (before any main process threads start). Once initialized, all
    /// methods are thread-safe for reading. THe static state is immutable after
    /// init() completes.
    ///
    /// Usage:
    ///
    /// ```cpp
    /// ResourcePath::init()  // Call once at startup (single-threaded)
    /// std::string_path = ResourcePath::resolve("res/img/icon.png")  // Thread-safe reads
    /// ```
    class ResourcePath {
    public:

        /// @brief Initialize the resource path system.
        /// Sets up base paths. Must be called before any resolve() calls.
        ///
        /// THREAD-SAFETY - Must be called from main thread before ThreadSystem starts.
        /// After initialization, all methods are safe for concurrent reads.
        static void init();

        /// @brief Resolve a relative resource path to an absolute path.
        /// Searches all registered paths in priority order.
        /// @param relative_path Path relative to resource root (e.g. "res/img/icon.png")
        /// @return Absolute path to the resource, or the original path if not found
        static std::string resolve(const std::string& relative_path);

        /// @brief Check if a resource exists at the given relative path.
        /// @param relative_path Path relative to resource root
        /// @return true if resource exists in any search path
        static bool exists(const std::string& relative_path);

        /// @brief Add a search path for resource resolution.
        /// Higher priority paths are searched first.
        /// @param path Absolute path to add as a search location
        /// @param priority Higher values = searched first (default: 0)
        static void addSearchPath(const std::string& path, int priority = 0);

        /// @brief Remove a previously added search path.
        /// @param path The path to remove
        static void removeSearchPath(const std::string& path);

        /// @brief Get the base resource path (primary search path).
        /// Useful for managers that need the root directory.
        /// @return The highest priority search path, or empty if not initialized
        static std::string getBasePath();

    private:
        struct ResourceSearchPath {
            std::string path;
            int priority;
        };

        static std::vector<ResourceSearchPath> search_paths_;
        static bool initialized_;

        static std::string getExecutablePath();
    };

} // namespace engine::utils
