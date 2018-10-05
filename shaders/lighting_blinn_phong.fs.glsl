#version 150 core

uniform vec4 lightPosES;

in vec4 v_pos;
in vec3 v_nrm;
in vec4 v_clr;

out vec4 color;

void main()
{
	const vec3 lightAmbient  = vec3(0.2);
	const vec3 lightDiffuse  = vec3(0.7);
	const vec3 lightSpecular = vec3(0.5);

	color.a = v_clr.a; // use input color's alpha, unlit
	color.rgb = lightAmbient * v_clr.rgb; // ambient part

	vec3 N = normalize(v_nrm); // re-normalize, interpolation can change length
	vec3 L = normalize(lightPosES.xyz - v_pos.xyz);

	float NdotL = dot(N,L);
	if (NdotL > 0.0) {
		color.rgb += NdotL * lightDiffuse * v_clr.rgb; // diffuse part
		vec3 E = normalize(-v_pos.xyz);
		vec3 H = normalize(E+L);
		float NdotH = dot(N,H);
		if (NdotH > 0.0) {
			color.rgb += pow(NdotH, 16.0) * lightSpecular * v_clr.rgb; // specular part
		}
	}
}
