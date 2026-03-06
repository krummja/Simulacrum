#include "resource_path.hpp"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <filesystem>
#include <format>

namespace Simulacrum
{

  namespace fs = std::filesystem;

  std::vector<ResourcePath::ResourceSearchPath> ResourcePath::search_paths_;
  bool ResourcePath::initialized_ = false;

  void ResourcePath::init()
  {
    if (initialized_)
    {
      return;
    }

    std::string base_path = getExecutablePath();

    // Normalize to generic format
    fs::path exe_path(base_path);
    std::string exe_path_str = exe_path.generic_string();

    fs::path bin_path(base_path);

    std::string bin_path_str = bin_path.generic_string();

    if (bin_path_str.find("Debug/") != std::string::npos ||
      bin_path_str.find("Release/") != std::string::npos ||
      bin_path_str.find("Profile/") != std::string::npos)
    {
      fs::path project_root = bin_path.parent_path().parent_path();
      addSearchPath(project_root.string(), 0);
    }
    else
    {
      addSearchPath(base_path, 0);
    }

    initialized_ = true;
    spdlog::info("Base path = {}", getBasePath());
  }

  std::string ResourcePath::getExecutablePath()
  {
    // SDL_GetBasePath returns the directory containing the executable
    // SDL3 returns const char* (static storage, no need to free)
    const char* base_path = SDL_GetBasePath();
    if (base_path && base_path[0] != '\0')
    {
      return std::string(base_path);
    }

    // Fallback to current working directory.
    return fs::current_path().string();
  }

  std::string ResourcePath::resolve(const std::string& relative_path)
  {
    if (!initialized_)
    {
      // Fallback to relative path if not initialized
      return relative_path;
    }

    // CHeck each search path in priority order (highest first)
    for (const auto& search_path : search_paths_)
    {
      fs::path full_path = fs::path(search_path.path) / relative_path;
      if (fs::exists(full_path))
      {
        return full_path.string();
      }
    }

    // Not found in any search path - return relative path as fallback
    // This alows the caller's error handling to report the missing resource
    return relative_path;
  }

  bool ResourcePath::exists(const std::string& relative_path)
  {
    if (!initialized_)
    {
      return fs::exists(relative_path);
    }

    for (const auto& search_path : search_paths_)
    {
      fs::path full_path = fs::path(search_path.path) / relative_path;
      if (fs::exists(full_path))
      {
        return true;
      }
    }

    return false;
  }

  void ResourcePath::addSearchPath(const std::string& path, int priority)
  {
    // Check if path already exists
    auto it = std::find_if(
      search_paths_.begin(),
      search_paths_.end(),
      [&path](const ResourceSearchPath& sp) { return sp.path == path; }
    );

    if (it != search_paths_.end())
    {
      // Update the priority if already exists
      it->priority = priority;
    }
    else
    {
      // Add new path
      search_paths_.push_back({ path, priority });
    }

    std::sort(
      search_paths_.begin(),
      search_paths_.end(),
      [](const ResourceSearchPath& a, const ResourceSearchPath& b)
      {
        return a.priority > b.priority;
      }
    );
  }

  void ResourcePath::removeSearchPath(const std::string& path)
  {
    search_paths_.erase(
      std::remove_if(
        search_paths_.begin(),
        search_paths_.end(),
        [&path](const ResourceSearchPath& sp) { return sp.path == path; }
      ),
      search_paths_.end()
    );
  }

  std::string ResourcePath::getBasePath()
  {
    if (search_paths_.empty())
    {
      return "";
    }

    return search_paths_.front().path;
  }

} // namespace Simulacrum
