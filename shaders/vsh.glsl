#version 330 core

layout (location = 0) in vec4 pos;

out vec4 f_color;

uniform float angle;

void main() {
	float s = sin(angle), c = cos(angle);
	
	gl_Position = vec4(
		pos.x * c - pos.y * s,
		pos.x * s + pos.y * c,
		pos.zw
	);
	f_color = gl_Position + vec4(.4);
}