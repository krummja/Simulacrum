#ifndef AUDIO_MANAGER_HPP_
#define AUDIO_MANAGER_HPP_

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <SDL3_mixer/SDL_mixer.h>

namespace engine::resource {

    class AudioManager final {
        friend class ResourceManager;

    public:
        AudioManager();
        ~AudioManager();

        AudioManager(const AudioManager&) = delete;
        AudioManager& operator=(const AudioManager&) = delete;
        AudioManager(AudioManager&&) = delete;
        AudioManager& operator=(AudioManager&&) = delete;

    private:
        struct SDLMixChunkDeleter {
            void operator()(Mix_Chunk* chunk) const {
                if (chunk) {
                    Mix_FreeChunk(chunk);
                }
            }
        };

        struct SDLMixMusicDeleter {
            void operator()(Mix_Music* music) const {
                if (music) {
                    Mix_FreeMusic(music);
                }
            }
        };

        std::unordered_map<std::string, std::unique_ptr<Mix_Chunk, SDLMixChunkDeleter>> sounds_;
        std::unordered_map<std::string, std::unique_ptr<Mix_Music, SDLMixMusicDeleter>> music_;

        Mix_Chunk* loadSound(std::string_view file_path);
        Mix_Chunk* getSound(std::string_view file_path);
        void unloadSound(std::string_view file_path);
        void clearSounds();

        Mix_Music* loadMusic(std::string_view file_path);
        Mix_Music* getMusic(std::string_view file_path);
        void unloadMusic(std::string_view file_path);
        void clearMusic();

        void clearAudio();
    };

} // namespace engine::resource

#endif // AUDIO_MANAGER_HPP_