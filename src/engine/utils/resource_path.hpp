#pragma once

#include <string>
#include <vector>

namespace engine::utils {

    class ResourcePath {
    public:
        static void init();

        static std::string resolve(const std::string& relative_path);

        static bool exists(const std::string& relative_path);

        static bool exists(const std::string& relative_path);

        static void addSearchPath(const std::string& path, int priority = 0);

        static void removeSearchPath(const std::string& path);

        static std::string getBasePath();

        static bool isRunningFromBundle();

    private:
        struct SearchPath {
            std::string path;
            int priority;
        };

        static std::vector<SearchPath> search_paths_;
        static bool initialized_;
        static bool is_bundle_;

        static void detectExecutionContext();
        static std::string getExecutablePath();
    };

} // namespace engine::utils
