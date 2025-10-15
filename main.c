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

	GLuint VAO, buffers[2];
	GLfloat const vertex[][2] = {
		{+0.0, +0.7},
		{-0.7, -0.7},
		{+0.7, -0.7}
	};
	GLubyte indices[] = {
		0, 1, 2
	};
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(2, buffers);
	
	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertex, vertex, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	GLuint program = createProgram("shaders/vsh.glsl", "shaders/fsh.glsl");

	glUseProgram(program);
	
	GLint angle_u = glGetUniformLocation(program, "angle");
	float angle_v = 0;
	
	for (;;) {
		SDL_Event e;
		
		while (SDL_PollEvent(&e))
			if (e.type == SDL_EVENT_QUIT)
				return 0;
		
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glUniform1f(angle_u, angle_v += 0.01);
		
		glDrawElements(
			GL_TRIANGLES,
			sizeof vertex / sizeof *vertex,
			GL_UNSIGNED_BYTE,
			NULL
		);
		
		SDL_GL_SwapWindow(window);
	}

	return 0;
}