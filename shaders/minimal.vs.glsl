#version 150 core

in vec3 pos;

void main()
{
	// just scale the [-1,1] cube so that it fits into the NDC frustum 
	gl_Position = vec4(0.5*pos, 1.0);
}
