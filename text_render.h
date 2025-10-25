#ifndef TEXT_RENDER_H
#define TEXT_RENDER_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include <GL/glew.h>
#include "common.h"

C_DECL_START

#define FONT_INIT {0}

typedef struct {
	FT_Library lib;
	FT_Face face;
} font_t;

typedef struct {
	GLuint texture;
	FT_Int top, left;
	int width, height;
	FT_Pos advance;
} font_char_t;

typedef struct {
	FT_ULong start;
	FT_ULong end;
	font_char_t const *array;
} font_atlas_t;

extern char const *font_error_str(FT_Error error);
extern char const *bitmap_pixel_mode_str(FT_Pixel_Mode pixel_mode);
extern FT_Error font_load(font_t *font, char const *filename);
extern void font_close(font_t *font);

C_DECL_END

#endif // TEXT_RENDER_H