#version 330 core

in vec2 o_tex;
out vec4 frag;

uniform vec3 u_color;
uniform sampler2D texture0;

void main() {
	float r = texture(texture0, o_tex).r;
	
	if (r <= 0)
		discard;
	
	frag = vec4(u_color.r * r, u_color.g * r, u_color.b * r, 0);
}