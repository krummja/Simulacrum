#include "audio_manager.hpp"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace engine::resource {

    AudioManager::AudioManager() {
        MIX_InitFlags flags = MIX_INIT_OGG | MIX_INIT_MP3;

        if ((Mix_Init(flags) & flags) != flags) {
            throw std::runtime_error("AudioManager error: Mix_Init failed: " + std::string(SDL_GetError()));
        }

        if (!Mix_OpenAudio(0, nullptr)) {
            Mix_Quit();
            throw std::runtime_error("AudioManager error: Mix_OpenAudio failed: " + std::string(SDL_GetError()));
        }
    }

    AudioManager::~AudioManager() {
        Mix_HaltChannel(-1);
        Mix_HaltMusic();

        clearSounds();
        clearMusic();

        Mix_CloseAudio();
        Mix_Quit();
    }

    Mix_Chunk* AudioManager::loadSound(std::string_view file_path) {
        auto it = sounds_.find(std::string(file_path));
        if (it != sounds_.end()) {
            return it->second.get();
        }

        spdlog::debug("Loading sounds effects: {}", file_path);

        Mix_Chunk* raw_chunk = Mix_LoadWAV(file_path.data());
        if (!raw_chunk) {
            spdlog::error("Loading sound effect failed: '{}': {}", file_path, SDL_GetError());
            return nullptr;
        }

        sounds_.emplace(file_path, std::unique_ptr<Mix_Chunk, SDLMixChunkDeleter>(raw_chunk));
        spdlog::debug("Successfully loaded and cacehd sound effects: {}", file_path);
        return raw_chunk;
    }

    Mix_Chunk* AudioManager::getSound(std::string_view file_path) {
        auto it = sounds_.find(std::string(file_path));
        if (it != sounds_.end()) {
            return it->second.get();
        }

        spdlog::warn("Sound effect '{}' cache not found, trying to load.", file_path);
        return loadSound(file_path);
    }

    void AudioManager::unloadSound(std::string_view file_path) {
        auto it = sounds_.find(std::string(file_path));
        if (it != sounds_.end()) {
            spdlog::debug("Unloaded sound effect: {}", file_path);
            sounds_.erase(it);
        }
        else {
            spdlog::warn("Tried unloading non-existent sound effect: {}", file_path);
        }
    }

    void AudioManager::clearSounds() {
        if (!sounds_.empty()) {
            spdlog::debug("Clearing all {} cached sound effects.", sounds_.size());
            sounds_.clear();
        }
    }

    Mix_Music* AudioManager::loadMusic(std::string_view file_path) {
        return nullptr;
    }

    Mix_Music* AudioManager::getMusic(std::string_view file_path) {
        return nullptr;
    }

    void AudioManager::unloadMusic(std::string_view file_path) {}

    void AudioManager::clearMusic() {}

    void AudioManager::clearAudio() {}

} // namespace engine::resource
