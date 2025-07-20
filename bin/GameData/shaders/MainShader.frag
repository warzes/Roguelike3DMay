#version 460 core

layout(location = 0) in vec3 FragPosition;
layout(location = 1) in vec3 FragColor;
layout(location = 2) in vec3 FragNormal;
layout(location = 3) in vec2 FragTexCoords;

layout(binding = 0) uniform sampler2D diffuseTex;
layout(binding = 1) uniform sampler2D specularTex;
layout(binding = 2) uniform sampler2D emissionTex;
layout(binding = 3) uniform sampler2D normalTex;
layout(binding = 4) uniform sampler2D depthTex;

layout(location = 0) out vec4 OutFragColor;

void main()
{
	vec4 textureDiffuse = texture(diffuseTex, FragTexCoords);
	if (textureDiffuse.a < 0.2)
		discard;

	OutFragColor = textureDiffuse;
}