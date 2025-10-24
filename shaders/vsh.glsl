#version 330 core

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 tex;

uniform struct {
	int x, y, w, h;
} coord = {0, 0, 100, 100};
uniform struct {
	int w, h;
} screen;

void main() {
	gl_Position = pos;
}