#version 150 core

in vec4 v_clr;

out vec4 color;

void main()
{
	color = sin(gl_FragCoord*0.1);
}
