#include <stdlib.h>
#include <GL/glew.h>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include "common.h"
#include "shader.h"

#define TEST(expr, ...) do { \
	if (!(expr)) { \
		SDL_LogCritical(0, __VA_ARGS__); \
		return 1; \
	} \
} while (0)
#define SDL_TEST(expr, str) TEST(expr, str " failed: %s", SDL_GetError())

static SDL_Window *window;
static SDL_GLContext ctx;

static void cleanup() {
	if (ctx)
		SDL_GL_DestroyContext(ctx);
	
	if (window)
		SDL_DestroyWindow(window);
	
	LOG("resources cleanup");
	
	SDL_Quit();
}

int main() {
	SDL_TEST(SDL_Init(SDL_INIT_VIDEO), "init video");
	
	// set attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	
	SDL_TEST(window = SDL_CreateWindow(
		"OpenGL window",
		500, 500,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	), "create window");
	
	atexit(cleanup);
	
	SDL_TEST(ctx = SDL_GL_CreateContext(window), "create context");
	
	GLenum err;
	
	TEST((err = glewInit()) == GLEW_OK, "failed to init glew: %s", glewGetErrorString(err));
	
	// try using adaptative vsync or normal vsync
	if (!SDL_GL_SetSwapInterval(-1) && !SDL_GL_SetSwapInterval(1))
		LOG("vsync not supported");

#define DEBUG_GL_VAR(value) LOG(#value ": %s", glGetString(value))
	DEBUG_GL_VAR(GL_VENDOR);
	DEBUG_GL_VAR(GL_RENDERER);
	DEBUG_GL_VAR(GL_VERSION);
	DEBUG_GL_VAR(GL_SHADING_LANGUAGE_VERSION);
#undef DEBUG_GL_VAR

	GLuint program = createProgram("shaders/vsh.glsl", "shaders/fsh.glsl");
	
	LOG("program object: %d", program);
	
	for (;;) {
		SDL_Event e;
		
		while (SDL_PollEvent(&e))
			if (e.type == SDL_EVENT_QUIT)
				return 0;
		
		glClearColor(1, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		SDL_GL_SwapWindow(window);
	}

	return 0;
}