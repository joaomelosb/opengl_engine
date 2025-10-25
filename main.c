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
	if (!SDL_GL_SetSwapInterval(1) && !SDL_GL_SetSwapInterval(-1))
		LOG("vsync not supported");

#define DEBUG_GL_VAR(value) LOG(#value ": %s", glGetString(value))
	DEBUG_GL_VAR(GL_VENDOR);
	DEBUG_GL_VAR(GL_RENDERER);
	DEBUG_GL_VAR(GL_VERSION);
	DEBUG_GL_VAR(GL_SHADING_LANGUAGE_VERSION);
#undef DEBUG_GL_VAR

	GLuint VAO, texture, buffers[2];
	GLfloat const vertex[][2][2] = {
		{{-1.0, +1.0}, {0, 0}},
		{{+1.0, +1.0}, {1, 0}},
		{{+1.0, -1.0}, {1, 1}},
		{{-1.0, -1.0}, {0, 1}}
	};
	GLubyte indices[] = {
		0, 1, 2,
		0, 2, 3
	};
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(2, buffers);
	glGenTextures(1, &texture);
	
	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	// texture params
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertex, vertex, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof *vertex, NULL);
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof *vertex, (void *)(sizeof **vertex));
	glEnableVertexAttribArray(1);

	GLuint program = createProgram("shaders/vsh.glsl", "shaders/fsh.glsl");

	glUseProgram(program);

#define DEF_UNIFORM(name) GLint name = glGetUniformLocation(program, #name)
	DEF_UNIFORM(u_screen);
	DEF_UNIFORM(u_pos);
	DEF_UNIFORM(u_size);
#undef DEF_UNIFORM
	
	int width, height;
	
	SDL_GetWindowSize(window, &width, &height);
	
	union {
		struct {
			float x, y;
		};
		float raw[2];
	} pos = {{100, 100}}, size;
	
	font_t font = FONT_INIT;
	FT_Error error;
	
	if ((error = font_load(&font, "Monaco.ttf"))) {
		LOG_ERR("couldn't load font: %s", font_error_str(error));
		return 1;
	}
	
	const FT_Bitmap *bitmap = NULL;
	
	font_set_size(&font, 300, 300);
	
	if ((error = font_load_char(&font, 'A', &bitmap))) {
		LOG("couldn't load char: %s", font_error_str(error));
		font_close(&font);
		return 1;
	}
	
	LOG("rows=%d, width=%d, pitch=%d, pixel_mode=%s", bitmap->rows,
		bitmap->width, bitmap->pitch, bitmap_pixel_mode_str(bitmap->pixel_mode));
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		bitmap->width,
		bitmap->rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		bitmap->buffer
	);
	
	size.x = bitmap->width;
	size.y = bitmap->rows;
	
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
		
		glViewport(0, 0, width, height);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glUniform2i(u_screen, width, height);
		glUniform2fv(u_pos, 1, pos.raw);
		glUniform2fv(u_size, 1, size.raw);
		
		glDrawElements(
			GL_TRIANGLES,
			sizeof indices / sizeof *indices,
			GL_UNSIGNED_BYTE,
			NULL
		);
		
		SDL_GL_SwapWindow(window);
	}

	return 0;
}