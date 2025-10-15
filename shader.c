#include <stdlib.h>
#include "load_file.h"
#include "shader.h"

typedef void (GLAPIENTRY *GETPARAMFN)(GLuint, GLenum, GLint *);
typedef void (GLAPIENTRY *GETINFOLOGFN)(GLuint, GLsizei, GLsizei *, GLchar *);

static GLint getGLObjectParam(
	GLuint obj,
	GLenum param,
	GETPARAMFN param_fn
) {
	GLint value;
	
	param_fn(obj, param, &value);
	
	return value;
}

#ifndef DISABLE_SHADER_DEBUG

static char *getGLObjectInfoLog(
	GLuint obj,
	GETPARAMFN param_fn,
	GETINFOLOGFN infolog_fn,
	GLint *log_len
) {
	GLint len = getGLObjectParam(obj, GL_INFO_LOG_LENGTH, param_fn);
	
	if (!len)
		return NULL;
	
	char *log_ptr = malloc(len);
	
	if (!log_ptr) {
		LOG("failed to allocate %d bytes for info log", len);
		return NULL;
	}
	
	infolog_fn(obj, *log_len = len, NULL, log_ptr);
	
	return log_ptr;
}

#endif // DISABLE_SHADER_DEBUG

GLuint loadShader(char const *filename, GLenum type) {
	GLuint sh = glCreateShader(type);
	
	if (!sh) {
		LOG("shader creation failed for shader \"%s\"", filename);
		return 0;
	}
	
	char *buf = load_file(filename, NULL);
	
	if (!buf) {
		glDeleteShader(sh);
		return 0;
	}
	
	glShaderSource(sh, 1, (const GLchar **)&buf, NULL);
	glCompileShader(sh);
	free(buf);
	
	char const *sh_type = (
		type == GL_VERTEX_SHADER ? "vertex" :
		type == GL_FRAGMENT_SHADER ? "fragment" :
		"unknown"
	);

#ifndef DISABLE_SHADER_DEBUG
	GLint log_len = 0;
	char *log_ptr = getGLObjectInfoLog(
		sh,
		glGetShaderiv,
		glGetShaderInfoLog,
		&log_len
	);
	
	if (log_ptr) {
		LOG("compile log of \"%s\" %s shader (%d bytes long):\n%s", filename,
			sh_type, log_len, log_ptr);
		free(log_ptr);
	}
#endif // DISABLE_SHADER_DEBUG

	if (!getGLObjectParam(sh, GL_COMPILE_STATUS, glGetShaderiv)) {
		LOG_ERR("failed to compile %s shader \"%s\"", sh_type, filename);
		glDeleteShader(sh);
		return 0;
	}
	
	return sh;
}

GLuint createProgram(char const *vsh_file, char const *fsh_file) {
	GLuint vsh, fsh;
	
	vsh = loadShader(vsh_file, GL_VERTEX_SHADER);
	fsh = loadShader(fsh_file, GL_FRAGMENT_SHADER);
	
	if (!vsh || !fsh) {
		if (vsh)
			glDeleteShader(vsh);
		
		if (fsh)
			glDeleteShader(fsh);
		
		return 0;
	}
	
	GLuint program = glCreateProgram();

	if (!program) {
		LOG("failed to create program (\"%s\", \"%s\")", vsh_file, fsh_file);
		return 0;
	}
	
	glAttachShader(program, vsh);
	glAttachShader(program, fsh);
	glLinkProgram(program);
	glDeleteShader(vsh);
	glDeleteShader(fsh);
	
#ifndef DISABLE_SHADER_DEBUG
	GLint log_len = 0;
	char *log_ptr = getGLObjectInfoLog(
		program,
		glGetProgramiv,
		glGetProgramInfoLog,
		&log_len
	);
	
	if (log_ptr) {
		LOG("link log of program (\"%s\", \"%s\") (%d bytes long):\n%s", vsh_file,
			fsh_file, log_len, log_ptr);
		free(log_ptr);
	}
#endif

	if (!getGLObjectParam(program, GL_LINK_STATUS, glGetProgramiv)) {
		LOG_ERR("failed to link program (\"%s\", \"%s\")", vsh_file, fsh_file);
		glDeleteProgram(program);
		return 0;
	}
	
	return program;
}