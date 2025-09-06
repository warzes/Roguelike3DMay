#version 460 core

layout(location = 0) in vec2 vTexCoords;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vWorldPosition;
layout(location = 4) in float visibility;

layout(binding = 0) uniform sampler2D diffuseTex;
layout(binding = 1) uniform sampler2D specularTex;
layout(binding = 2) uniform sampler2D emissionTex;
layout(binding = 3) uniform sampler2D normalTex;
layout(binding = 4) uniform sampler2D depthTex;

layout(location = 0) out vec4 OutFragColor;

layout(binding = 0, std140) uniform SceneUniforms { 
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 eyePosition;
	float fogStart;
	float fogEnd;
	vec3 fogColor;
};

layout(binding = 3, std140) uniform MaterialUniforms { 
	vec4 diffuse;

	bool hasDiffuseTexture;
	bool hasSpecularTexture;
	bool hasEmissionTexture;
	bool hasNormalMapTexture;
	bool hasDepthMapTexture;

	bool noLighing;
};

void main()
{
	// определение diffuse, либо с текстуры, либо с материала
	vec4 DiffuseColor = (hasDiffuseTexture)
		? texture(diffuseTex, vTexCoords)
		: diffuse;
	DiffuseColor = DiffuseColor * vColor;

	// прозрачность скипаем
	if (DiffuseColor.a < 0.02)
		discard;

	// на объете не использовать освещение и тени (то есть чистый diffuseColor)
	if (noLighing)
	{
		OutFragColor = DiffuseColor;
		return;
	}

	OutFragColor.rgb = mix(fogColor, DiffuseColor.rgb, visibility);
	//OutFragColor.rgb = round(OutFragColor.rgb * 64.0) / 64.0; // эффект как в варлоке
	OutFragColor.a = DiffuseColor.a;
}