#include <stdlib.h>
#include "text_render.h"
#include "shader.h"
#include "config.h"

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

void font_close(font_t *font) {
	if (font->face)
		FT_Done_Face(font->face);
	if (font->lib)
		FT_Done_FreeType(font->lib);
}

FT_Error font_atlas_load(font_t const *font, font_atlas_t *atlas, FT_Int size) {
	if (atlas->start >= atlas->end)
		return FT_Err_Invalid_Argument;
	
	FT_ULong count = atlas->end - atlas->start + 1;
	
	font_char_t *array = malloc(sizeof *array * count);
	
	if (!array)
		return FT_Err_Out_Of_Memory;
	
	FT_Set_Pixel_Sizes(font->face, size, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	for (FT_ULong i = 0; i < count; i++) {
		FT_Error error;
		
		if ((error = FT_Load_Char(font->face, atlas->start + i, FT_LOAD_RENDER))) {
			free(array);
			return error;
		}
		
		FT_GlyphSlot glyph = font->face->glyph;
		const FT_Bitmap *bitmap = &glyph->bitmap;
		font_char_t *ch = array + i;
		
		ch->top = glyph->bitmap_top;
		ch->left = glyph->bitmap_left;
		ch->width = bitmap->width;
		ch->height = bitmap->rows;
		ch->advance = glyph->advance.x;
		
		glGenTextures(1, &ch->texture);
		glBindTexture(GL_TEXTURE_2D, ch->texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ch->width, ch->height, 0, GL_RED, GL_UNSIGNED_BYTE,
			bitmap->buffer);
	}
	
	atlas->array = array;
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	return 0;
}

void font_atlas_free(font_atlas_t *atlas) {
	free(atlas->array);
}

font_char_t *font_atlas_get_ch(font_atlas_t const *atlas, FT_ULong code) {
	if (code < atlas->start || code > atlas->end)
		return NULL;
	
	return atlas->array + (code - atlas->start);
}

static GLuint get_quad_VAO() {
	static GLuint VAO;
	
	if (VAO)
		return VAO;
	
	GLuint VBO;
	GLfloat const vertex[][2][2] = {
		{{-1.0, +1.0}, {0, 0}},
		{{+1.0, +1.0}, {1, 0}},
		{{+1.0, -1.0}, {1, 1}},
		{{-1.0, -1.0}, {0, 1}},
		{{-1.0, +1.0}, {0, 0}},
		{{+1.0, -1.0}, {1, 1}},
		{{-1.0, -1.0}, {0, 1}}
	};
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof vertex, vertex, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof *vertex, NULL);
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof *vertex, (void *)(sizeof **vertex));
	glEnableVertexAttribArray(1);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	return VAO;
}

bool text_ctx_init(text_ctx_t *ctx) {
	ctx->program = createProgram(CONFIG_FONT_VSH, CONFIG_FONT_FSH);
	
	if (!ctx->program)
		return false;
	
	ctx->VAO = get_quad_VAO();
	
#define DEF_UNIFORM(name) if ((ctx->name = glGetUniformLocation(ctx->program, #name)) == -1) { \
	LOG("couldn't find uniform \"" #name "\""); \
	return false; \
}
	DEF_UNIFORM(u_screen);
	DEF_UNIFORM(u_size);
	DEF_UNIFORM(u_pos);
	DEF_UNIFORM(u_color);
#undef DEF_UNIFORM

	return true;
}

static inline void draw_text_cleanup() {
	glUseProgram(0);
	glBindVertexArray(0);
}

bool draw_text(
	text_ctx_t const *ctx,
	char const *str,
	int x,
	int y,
	int vw,
	int vh,
	float color[3]
) {
	glBindVertexArray(ctx->VAO);
	glUseProgram(ctx->program);
	
	glUniform2i(ctx->u_screen, vw, vh);
	glUniform3fv(ctx->u_color, 1, color);
	glActiveTexture(GL_TEXTURE0);
	
	for (FT_Pos advance = 0; *str; str++) {
		font_char_t *ch = font_atlas_get_ch(ctx->atlas, *str);
		
		if (!ch) {
			LOG("'%c' is out of atlas range", *str);
			draw_text_cleanup();
			return false;
		}
		
		glBindTexture(GL_TEXTURE_2D, ch->texture);
		glUniform2f(ctx->u_size, ch->width, ch->height);
		glUniform2f(ctx->u_pos, x + ch->left + advance, y - ch->top);
		glDrawArrays(GL_TRIANGLES, 0, 7);
		
		advance += ch->advance >> 6;
	}
	
	draw_text_cleanup();
	
	return true;
}