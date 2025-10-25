#ifndef TEXT_RENDER_H
#define TEXT_RENDER_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdbool.h>
#include "common.h"

C_DECL_START

typedef struct {
	FT_Library lib;
	FT_Face face;
} font_t;

#define FONT_INIT {0}

extern char const *font_error_str(FT_Error error);
extern char const *bitmap_pixel_mode_str(FT_Pixel_Mode pixel_mode);
extern FT_Error font_load(font_t *font, char const *filename);
extern FT_Error font_set_size(font_t *font, FT_UInt width, FT_UInt height);
extern FT_Error font_load_char(font_t const *font, FT_ULong code, const FT_Bitmap **bitmap_ptr);
extern void font_close(font_t *font);

C_DECL_END

#endif // TEXT_RENDER_H