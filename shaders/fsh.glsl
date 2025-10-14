#version 330 core

out vec4 frag;
in vec2 a;

void main() {
	frag = vec4(1) * vec4(a, 0, 1);
}