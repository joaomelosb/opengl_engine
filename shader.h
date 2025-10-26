#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include "common.h"

C_DECL_START

extern GLuint loadShader(char const *, GLenum);

extern GLuint createProgram(char const *, char const *);

C_DECL_END

#endif