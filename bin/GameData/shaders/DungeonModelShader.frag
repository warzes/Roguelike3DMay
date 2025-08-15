#version 460 core

layout(location = 0) in vec2 vTexCoords;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vViewDir;
layout(location = 4) in vec3 vWorldPosition;
layout(location = 5) in vec3 vCameraPosition;
layout(location = 6) in vec4 lightVertexColor;
layout(location = 7) in float visibility;

layout(binding = 0) uniform sampler2D diffuseTex;
layout(binding = 1) uniform sampler2D specularTex;
layout(binding = 2) uniform sampler2D emissionTex;
layout(binding = 3) uniform sampler2D normalTex;
layout(binding = 4) uniform sampler2D depthTex;

layout(location = 0) out vec4 OutFragColor;

layout(binding = 0, std140) uniform SceneUniforms { 
	mat4 viewMatrix;
	mat4 projectionMatrix;
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

// глобальные переменные
vec4 DiffuseColor;

void main()
{
	// определение diffuse, либо с текстуры, либо с материала
	DiffuseColor = diffuse * vec4(vColor, 1.0);
	if (hasDiffuseTexture)
	{
		DiffuseColor = texture(diffuseTex, vTexCoords);
	}

	// прозрачность скипаем
	if (DiffuseColor.a < 0.2)
		discard;

	// на объете не использовать освещение и тени (то есть чистый diffuseColor)
	if (noLighing)
	{
		OutFragColor = DiffuseColor;
		return;
	}

	//OutFragColor = DiffuseColor;
	DiffuseColor *= lightVertexColor;
	OutFragColor.rgb = mix(fogColor, DiffuseColor.rgb, visibility);
	//OutFragColor += vec4(texture2D(ditherTexture, gl_FragCoord.xy / 8.0).r / 32.0 - (1.0 / 128.0));//dithering
	OutFragColor.rgb = round(OutFragColor.rgb * 64) / 64;
	OutFragColor.a = DiffuseColor.a;
}