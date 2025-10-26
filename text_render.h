#ifndef TEXT_RENDER_H
#define TEXT_RENDER_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdbool.h>
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
	font_char_t *array;
} font_atlas_t;

typedef struct {
	GLuint VAO;
	GLuint program;
	GLint u_screen;
	GLint u_size;
	GLint u_pos;
	GLint u_color;
	font_atlas_t const *atlas;
} text_ctx_t;

// freetype utils
extern char const *font_error_str(FT_Error error);

extern char const *bitmap_pixel_mode_str(FT_Pixel_Mode pixel_mode);

// handle fonts
extern FT_Error font_load(font_t *font, char const *filename);

extern void font_close(font_t *font);

extern FT_Error font_atlas_load(font_t const *font, font_atlas_t *atlas, FT_Int size);

extern void font_atlas_free(font_atlas_t *atlas);

extern font_char_t *font_atlas_get_ch(font_atlas_t const *atlas, FT_ULong code);

// text drawing
bool text_ctx_init(text_ctx_t *ctx);

bool draw_text(text_ctx_t const *ctx, char const *str, int x, int y, int vw, int vh, float color[3]);

C_DECL_END

#endif // TEXT_RENDER_H