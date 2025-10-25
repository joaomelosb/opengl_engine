LIBS := -lSDL3 -lglew32 -lopengl32 -lfreetype -L.
ARGUMENTS := -Wall -Wextra -pedantic-errors
FILES := main.c shader.c load_file.c text_render.c

all:
	gcc $(FILES) $(ARGUMENTS) $(LIBS)