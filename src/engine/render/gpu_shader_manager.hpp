#pragma once

#include <SDL3/SDL_gpu.h>
#include <string>
#include <unordered_map>

namespace engine::render {

    /// @brief Shader resource information for creation.
    struct ShaderInfo {
        uint32_t num_samplers{0};
        uint32_t num_storage_textures{0};
        uint32_t num_storage_buffers{0};
        uint32_t num_uniform_buffers{0};
    };

    /// @brief Singleton manager for GPU shaders.
    /// Handles loading of SPIR-V (Vulkan) shaders.
    class GPUShaderManager {
    public:
        static GPUShaderManager& Instance();

        /// @brief Initialize the shader manager.
        /// @param device GPU device
        /// @return true on success
        bool init(SDL_GPUDevice* device);

        /// @brief Shutdown and release all shaders.
        void shutdown();

        /// @brief Load a shader from file.
        ///
        /// @param path Shader path (e.g. "res/shaders/sprite.vert")
        /// @param stage Shader stage (VERTEX or FRAGMENT)
        /// @param info Resource binding information
        /// @return Loaded shader, or nullptr on failure
        SDL_GPUShader* loadShader(
            const std::string& path,
            SDL_GPUShaderStage stage,
            const ShaderInfo& info
        );

        /// @brief Get a previously loaded shader by name.
        /// @param name Shader name (usually the path used during loading)
        /// @return Shader pointer, or nullptr if not found
        SDL_GPUShader* getShader(const std::string& name) const;

        /// @brief Check if a shader is already loaded.
        bool hasShader(const std::string& name) const;

    private:
        GPUShaderManager() = default;
        ~GPUShaderManager() = default;

        // Non-copyable
        GPUShaderManager(const GPUShaderManager&) = delete;
        GPUShaderManager& operator=(const GPUShaderManager&) = delete;

        // Load SPIRV binary shader
        SDL_GPUShader* load_SPIRV(
            const std::string& path,
            SDL_GPUShaderStage stage,
            const ShaderInfo& info
        );

        SDL_GPUDevice* device_{nullptr};
        std::unordered_map<std::string, SDL_GPUShader*> shaders_;
    };

} // engine::render
