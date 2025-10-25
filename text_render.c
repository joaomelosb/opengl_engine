#include "text_render.h"

char const *font_error_str(FT_Error error) {
#undef FTERRORS_H_
#define FT_ERROR_START_LIST switch (error) {
#define FT_ERRORDEF(n, v, s) case n: return s;
#define FT_ERROR_END_LIST }
#include <freetype/fterrors.h>
	return "unknown";
}

char const *bitmap_pixel_mode_str(FT_Pixel_Mode pixel_mode) {
#define _CASE(c) case c: return #c;
#define CASE(c) _CASE(FT_PIXEL_MODE_##c)
	switch (pixel_mode) {
	CASE(NONE)
	CASE(MONO)
	CASE(GRAY)
	CASE(GRAY2)
	CASE(GRAY4)
	CASE(LCD)
	CASE(LCD_V)
	CASE(BGRA)
	CASE(MAX)
	}
	
	return "UNKNOWN";
#undef _CASE
#undef CASE
}

FT_Error font_load(font_t *font, char const *filename) {
	FT_Error error;
	
	if ((error = FT_Init_FreeType(&font->lib)))
		return error;
	
	if ((error = FT_New_Face(font->lib, filename, 0, &font->face))) {
		FT_Done_FreeType(font->lib);
		return error;
	}
	
	return 0;
}

FT_Error font_set_size(font_t *font, FT_UInt width, FT_UInt height) {
	return FT_Set_Pixel_Sizes(font->face, width, height);
}

FT_Error font_load_char(font_t const *font, FT_ULong code, const FT_Bitmap **bitmap_ptr) {
	FT_Error error;
	
	if ((error = FT_Load_Char(font->face, code, FT_LOAD_RENDER)))
		return error;
	
	*bitmap_ptr = &font->face->glyph->bitmap;
	
	return 0;
}

void font_close(font_t *font) {
	if (font->face)
		FT_Done_Face(font->face);
	if (font->lib)
		FT_Done_FreeType(font->lib);
}