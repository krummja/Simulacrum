#include <SDL3/SDL.h>
#include "ResourcePath.hpp"

namespace Simulacrum
{
  SDL_Surface* LoadPNGTexture(const char* file_name, int channels) {
    char full_path[256];
    SDL_Surface* result;
    SDL_PixelFormat format;

    const std::string res_path = ResourcePath::resolve("res");
    SDL_snprintf(full_path, sizeof(full_path), "%s/img/%s", res_path, file_name);
    result = SDL_LoadPNG(full_path);

    if (channels == 4)
    {
      format = SDL_PIXELFORMAT_ABGR8888;
    }

    else
    {
      SDL_assert(!"Unexpected channels");
      SDL_DestroySurface(result);
      return nullptr;
    }

    if (result->format != format)
    {
      SDL_Surface* next = SDL_ConvertSurface(result, format);
      SDL_DestroySurface(result);
      result = next;
    }

    return result;
  }

}
