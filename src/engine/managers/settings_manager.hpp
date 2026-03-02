#pragma once

#include "macros.hpp"

#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Simulacrum {

    class Simulacrum_API SettingsManager {
    public:
        ~SettingsManager() = default;

        /// @brief Gets the singleton instance of the SettingsManager
        /// @return Reference to the SettingsManager singleton instance
        static SettingsManager& Instance() {
            static SettingsManager instance;
            return instance;
        }

        /// @brief Supported setting value types
        using SettingValue = std::variant<int, float, bool, std::string>;

        /// @brief Callback function type for change notifications
        /// @param category The category that changed
        /// @param key The setting key that changed
        /// @param newValue The new value of the setting
        using ChangeCallback = std::function<void(
            const std::string& category,
            const std::string& key,
            const SettingValue& newValue
        )>;

        bool loadFromFile(const std::string& filepath);

        // bool saveToFile(const std::string& filepath);

        template<typename T>
        T get(const std::string& category, const std::string& key, T default_value = T{}) const;

        template<typename T>
        bool set(const std::string& category, const std::string& key, const T& value);

        // bool has(const std::string& category, const std::string& key) const;

        // bool remove(const std::string& category, const std::string& key);

        // bool clearCategory(const std::string& category);

        // void clearAll();

        // size_t registerChangeListener(const std::string& category, ChangeCallback callback);

        // void unregisterChangeListener(size_t callback_id);

        // void getCategories(std::vector<std::string>& outCategories) const;

        // void getKeys(const std::string& category, std::vector<std::string>& out_keys) const;

    private:
        using CategorySettings = std::unordered_map<std::string, SettingValue>;
        std::unordered_map<std::string, CategorySettings> settings_;

        mutable std::shared_mutex settings_mutex_;

        // struct ListenerInfo {
        //     size_t id;
        //     std::string category;
        //     ChangeCallback callback;
        // };
        // std::vector<ListenerInfo> listeners_;
        // mutable std::mutex listeners_mutex_;
        // size_t next_callback_id_ = 0;

        // void notifyListeners(const std::string& category, const std::string&key, const SettingValue& new_value);

        // std::string variantToString(const SettingValue& value) const;

        SettingsManager(const SettingsManager&) = delete;
        SettingsManager& operator=(const SettingsManager&) = delete;

        SettingsManager() = default;
    };

    template<typename T>
    T SettingsManager::get(const std::string& category, const std::string& key, T default_value) const {
        std::shared_lock<std::shared_mutex> lock(settings_mutex_);

        auto category_it = settings_.find(category);
        if (category_it == settings_.end()) {
            return default_value;
        }

        auto key_it = category_it->second.find(key);
        if (key_it == category_it->second.end()) {
            return default_value;
        }

        try {
            if constexpr (std::is_same_v<T, int>) {
                return std::get<int>(key_it->second);
            } else if constexpr (std::is_same_v<T, float>) {
                return std::get<float>(key_it->second);
            } else if constexpr (std::is_same_v<T, bool>) {
                return std::get<bool>(key_it->second);
            } else if constexpr (std::is_same_v<T, std::string>) {
                return std::get<std::string>(key_it->second);
            } else {
                return default_value;
            }
        } catch (const std::bad_variant_access) {
            return default_value;
        }
    }

    template<typename T>
    bool SettingsManager::set(const std::string& category, const std::string& key, const T& value) {
        SettingValue setting_value;

        if constexpr (std::is_same_v<T, int>) {
            setting_value = value;
        } else if constexpr (std::is_same_v<T, float>) {
            setting_value = value;
        } else if constexpr (std::is_same_v<T, bool>) {
            setting_value = value;
        } else if constexpr (std::is_same_v<T, std::string>) {
            setting_value = value;
        } else if constexpr (std::is_convertible_v<T, std::string>) {
            setting_value = std::string(value);
        } else {
            return false;
        }

        {
            std::unique_lock<std::shared_mutex> lock(settings_mutex_);
            settings_[category][key] = setting_value;
        }

        // notifyListeners(category, key, setting_value);

        return true;
    }

}
