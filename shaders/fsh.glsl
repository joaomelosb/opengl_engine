#version 330 core

in vec2 o_tex;
out vec4 frag;

uniform sampler2D texture0;

void main() {
	float r = texture(texture0, o_tex).r;
	frag = vec4(1 * r, 1 * r, 1 * r, 0);
}