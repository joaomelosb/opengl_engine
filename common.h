#ifndef COMMON_H
#define COMMON_H

#include <SDL3/SDL.h>

#ifdef __cplusplus
#define C_DECL_START extern "C" {
#define C_DECL_END }
#else
#define C_DECL_START
#define C_DECL_END
#endif

#define LOG(...) SDL_Log("LOG: " __VA_ARGS__)
#define LOG_ERR(...) SDL_LogCritical(0, __VA_ARGS__)

#endif