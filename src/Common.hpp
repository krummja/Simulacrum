#ifndef SIMULACRUM_COMMON_HPP_
#define SIMULACRUM_COMMON_HPP_

#include <SDL3/SDL.h>

namespace Simulacrum
{
  SDL_Surface* LoadPNGTexture(const char* file_name, int channels);
}

#endif