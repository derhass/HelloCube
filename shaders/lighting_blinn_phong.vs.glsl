#version 150 core

uniform mat4 modelView;
uniform mat4 projection;
uniform mat3 normalMatrix;

in vec3 pos;
in vec3 nrm;
in vec4 clr;

out vec4 v_pos; // eye-space position
out vec3 v_nrm; // eye-space normal
out vec4 v_clr;

void main()
{
	v_pos = modelView * vec4(pos, 1.0);
	v_nrm = normalize(normalMatrix * nrm);
	v_clr = clr;
	gl_Position = projection * v_pos;
}
