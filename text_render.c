#include <stdlib.h>
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

FT_Error font_load_atlas(font_t const *font, font_atlas_t *atlas, FT_Int size) {
	if (atlas->start >= atlas->end)
		return FT_Err_Invalid_Argument;
	
	FT_ULong count = atlas->end - atlas->start + 1;
	
	font_char_t *array = malloc(sizeof *array * count);
	
	if (!array)
		return FT_Err_Out_Of_Memory;
	
	FT_Set_Pixel_Sizes(font->face, size, 0);
	
	for (FT_ULong i = 0; i < count; i++) {
		FT_Error error;
		
		if ((error = FT_Load_Char(font->face, atlas->start + i, FT_LOAD_RENDER))) {
			free(array);
			return error;
		}
		
		font_chat_t *ch = array + i;
		const FT_Bitmap *bitmap = &font->face->glyph->bitmap;
		
		ch->top = face->glyph->bitmap_top;
		ch->left = face->glyph->bitmap_left;
		ch->width = bitmap->width;
		ch->height = bitmap->rows;
		ch->advance = bitmap->advance.x;
		
		glGenTextures(1, &ch->texture);
		glBindTexture(GL_TEXTURE_2D, ch->texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ch->width, ch->height, 0, GL_RED, GL_UNSIGNED_BYTE,
			bitmap->buffer);
	}
	
	atlas->array = array;
	
	return 0;
}

void font_close(font_t *font) {
	if (font->face)
		FT_Done_Face(font->face);
	if (font->lib)
		FT_Done_FreeType(font->lib);
}