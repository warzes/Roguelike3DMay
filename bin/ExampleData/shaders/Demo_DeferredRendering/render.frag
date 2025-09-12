#version 460 core

in VS_OUT
{
	vec3 worldPosition;
	vec3 normal;
	vec3 tangent;
	vec2 texcoord0;
} fs_in;

layout(binding = 0) uniform sampler2D diffuseTexture;
//layout(binding = 1) uniform sampler2D specularTexture;
//layout(binding = 2) uniform sampler2D emissionTexture;
layout(binding = 3) uniform sampler2D textureNormal;
//layout(binding = 4) uniform sampler2D depthTexture;

layout(location = 0) out vec4 outputColor0; // diffusw
layout(location = 1) out vec3 outputColor1; // normal
layout(location = 2) out vec4 outputColor2;

void main()
{
	vec3 N = normalize(fs_in.normal);
	vec3 T = normalize(fs_in.tangent);
	vec3 B = cross(N, T);
	mat3 TBN = mat3(T, B, N);

	vec3 nm = texture(textureNormal, fs_in.texcoord0).xyz * 2.0 - vec3(1.0);
	nm = TBN * normalize(nm);

	vec4 color = texture(diffuseTexture, fs_in.texcoord0);

	outputColor0 = color;
	outputColor1 = nm;
	outputColor2.xyz = fs_in.worldPosition;
	outputColor2.w = 60.0f;
}