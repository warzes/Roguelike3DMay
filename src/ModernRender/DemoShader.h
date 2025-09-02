#pragma once

const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec2 vertexTexCoord;
layout(location = 4) in vec3 vertexTangent;

layout(binding = 0, std140) uniform SceneBlock {
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

layout(binding = 1, std140) uniform ModelMatricesBlock {
	mat4 modelMatrix;
};

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec4 fragWorldPosition;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);

	// Output all out variables
	fragTexCoord      = vertexTexCoord;
	fragNormal        = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
	fragWorldPosition = modelMatrix * vec4(vertexPosition, 1.0);
}
)";

const char* shaderCodeFragment = R"(
#version 460 core

//=============================================================
// AmbientLight.glsl
//=============================================================

struct AmbientLight
{
	vec3 color;
	bool isOn;
};

vec3 getAmbientLightColor(AmbientLight ambientLight)
{
	return ambientLight.isOn ? ambientLight.color : vec3(0.0, 0.0, 0.0);
}

//=============================================================
// DiffuseLight.glsl
//=============================================================

struct DiffuseLight
{
	vec3 color;
	vec3 direction;
	float factor;
	bool isOn;
};

vec3 getDiffuseLightColor(DiffuseLight diffuseLight, vec3 normal)
{
	if(!diffuseLight.isOn) {
		return vec3(0.0, 0.0, 0.0);
	}

	float finalIntensity = max(0.0, dot(normal, -diffuseLight.direction));
	finalIntensity = clamp(finalIntensity*diffuseLight.factor, 0.0, 1.0);
	return vec3(diffuseLight.color*finalIntensity);
}

//=============================================================
// SpecularHighlight.glsl
//=============================================================

struct Material
{
	bool isEnabled;
	float specularIntensity;
	float specularPower;
};

vec3 getSpecularHighlightColor(vec3 worldPosition, vec3 normal, vec3 eyePosition, Material material, DiffuseLight diffuseLight)
{
	if(!material.isEnabled) {
		return vec3(0.0);
	}

	vec3 reflectedVector = normalize(reflect(diffuseLight.direction, normal));
	vec3 worldToEyeVector = normalize(eyePosition - worldPosition);
	float specularFactor = dot(worldToEyeVector, reflectedVector);

	if (specularFactor > 0)
	{
		specularFactor = pow(specularFactor, material.specularPower);
		return diffuseLight.color * material.specularIntensity * specularFactor;
	}

	return vec3(0.0);
}

//=============================================================
// PointLight.glsl
//=============================================================

struct PointLight
{
	vec3 position;
	vec3 color;
	
	float ambientFactor;

	float constantAttenuation;
	float linearAttenuation;
	float exponentialAttenuation;
	
	bool isOn;
};

vec3 getPointLightColor(const PointLight pointLight, const vec3 worldPosition, const vec3 normal)
{
	if(!pointLight.isOn) {
		return vec3(0.0);
	}

	/*vec3 lightDir = normalize(pointLight.position - worldPosition);
	float diffuseFactor = max(0.0, dot(normal, lightDir));

	float distance = length(pointLight.position - worldPosition);

	float attenuation = pointLight.constantAttenuation
		+ pointLight.linearAttenuation * distance
		+ pointLight.exponentialAttenuation * distance * distance;

	vec3 lightColor = pointLight.color * diffuseFactor / attenuation;

	return lightColor;*/
	
	vec3 positionToLightVector = worldPosition - pointLight.position;
	float distance = length(positionToLightVector);
	positionToLightVector = normalize(positionToLightVector);
	
	float diffuseFactor = max(0.0, dot(normal, -positionToLightVector));
	float totalAttenuation = pointLight.constantAttenuation
		+ pointLight.linearAttenuation * distance
		+ pointLight.exponentialAttenuation * pow(distance, 2.0);

	return pointLight.color * (pointLight.ambientFactor + diffuseFactor) / totalAttenuation;
}

//=============================================================
// Fragment Code
//=============================================================

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragWorldPosition;

layout(binding = 0) uniform sampler2D diffuseTexture;

layout(binding = 2, std140) uniform LightSceneBlock {
	vec4 color; // TODO: diffuse
	AmbientLight ambientLight;
	DiffuseLight diffuseLight;
	Material material;
	vec3 eyePosition;
};

layout(binding = 3, std140) uniform PointLightsBlock {
	int numPointLights;
	PointLight pointLights[100];
};

layout(location = 0) out vec4 outputColor;

void main()
{
	vec3 normal = normalize(fragNormal);
	vec4 textureColor = texture(diffuseTexture, fragTexCoord);
	vec4 objectColor = textureColor*color;
	
	vec3 ambientColor = getAmbientLightColor(ambientLight);
	vec3 diffuseColor = getDiffuseLightColor(diffuseLight, normal);
	vec3 specularHighlightColor = getSpecularHighlightColor(fragWorldPosition.xyz, normal, eyePosition, material, diffuseLight);
	vec3 lightColor = ambientColor + diffuseColor + specularHighlightColor;

	for(int i = 0; i < numPointLights; i++) {
		lightColor += getPointLightColor(pointLights[i], fragWorldPosition.xyz, normal);
	}

	outputColor = objectColor * vec4(lightColor, 1.0);

//outputColor = vec4(fragNormal * 0.5 + 0.5, 1.0);
}
)";