#version 330 core

in vec4 pos;
out vec3 a;

void main() {
	vec3 a;
	gl_Position = pos;
}