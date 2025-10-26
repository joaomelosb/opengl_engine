#include <stdlib.h>
#include <GL/glew.h>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include "common.h"
#include "shader.h"
#include "text_render.h"

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
	
	font_t font;
	FT_Error error;
	
	if ((error = font_load(&font, "dina.ttc"))) {
		LOG("couldn't load font: %s", font_error_str(error));
		return 1;
	}
	
	font_atlas_t atlas = {.start = 32, .end = 255};
	const FT_Int font_size = 25;
	
	if ((error = font_atlas_load(&font, &atlas, font_size))) {
		LOG("couldn't load atlas: %s", font_error_str(error));
		font_close(&font);
		return 1;
	}
	
	text_ctx_t ctx = {.atlas = &atlas};
	
	if (!text_ctx_init(&ctx))
		return 1;
	
	char log_strings[4][1024];
	
#define WRITE_VAR(value, i) sprintf(log_strings[i], #value ": %s", glGetString(value))
	WRITE_VAR(GL_VENDOR, 0);
	WRITE_VAR(GL_RENDERER, 1);
	WRITE_VAR(GL_VERSION, 2);
	WRITE_VAR(GL_SHADING_LANGUAGE_VERSION, 3);
#undef WRITE_VAR

	char log_buf[128];
	unsigned frames = 0, frame_rate = 0;
	Uint64 lastTime = 0, lastFrameTime = 0, current;
	float color[] = {1, 1, 1};
	
	int width, height;
	SDL_GetWindowSize(window, &width, &height);
	
	for (;;) {
		SDL_Event e;
		
		while (SDL_PollEvent(&e))
			switch (e.type) {
			case SDL_EVENT_QUIT:
				return 0;
			case SDL_EVENT_WINDOW_RESIZED:
				width = e.window.data1;
				height = e.window.data2;
			}
		
		current = SDL_GetTicks();
		frames++;
		
		if (!lastTime)
			lastTime = current;
		
		if (current - lastTime >= 1000) {
			frame_rate = frames;
			frames = 0;
			lastTime = current;
		}
		
		if (!lastFrameTime)
			lastFrameTime = current;
		
		snprintf(log_buf, sizeof log_buf, "FPS: %u, delta time: %f", frame_rate,
			(current - lastFrameTime) / 1000.0);
		lastFrameTime = current;
		
		glViewport(0, 0, width, height);
		glClearColor(.05, .05, .05, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		size_t i = 0;
		FT_Int offset = font_size + 10;
		
		for (; i < sizeof log_strings / sizeof *log_strings; i++)
			draw_text(&ctx, log_strings[i], 10, offset * i + offset, width, height, color);
		
		draw_text(&ctx, log_buf, 10, offset * i + offset, width, height, color);
		
		SDL_GL_SwapWindow(window);
	}

	return 0;
}