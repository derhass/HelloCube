#version 150 core

in vec4 v_clr;
in vec3 v_pos;

out vec4 color;

void main()
{
	if(length(v_pos) < 1.4)
		discard;
	color = v_clr;
}
