#version 330 core

precision highp float;

layout (location = 0) in vec4 v_pos;
layout (location = 1) in vec2 v_tex;

out vec2 o_tex;

uniform ivec2 u_screen;
uniform vec2 u_pos;
uniform vec2 u_size;

void main() {
	vec2 pos_2d = v_pos.xy * (u_size / u_screen) + vec2(
		-1 + u_size.x / u_screen.x + u_pos.x / u_screen.x * 2,
		+1 - u_size.y / u_screen.y - u_pos.y / u_screen.y * 2
	);
	
	gl_Position = vec4(pos_2d, v_pos.zw);
	o_tex = v_tex;
}