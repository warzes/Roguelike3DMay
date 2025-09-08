#version 460 core

#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragWorldPosition;
layout(location = 3) in vec3 fragCameraPosition;

struct DirectionalLight {
	vec3 direction;
	vec3 color;
	float intensity;
	mat4 lightSpaceMatrix;
};

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
	vec3 attenuation; // x: constant, y: linear, z: quadratic
};

struct SpotLight {
	vec3 position;
	vec3 direction;
	vec3 color;
	float intensity;
	vec3 attenuation; // x: constant, y: linear, z: quadratic
	vec2 angles; // x: inner cutoff angle, y: outer cutoff angle (for spotlight)
};

layout(binding = 2, std140) uniform LightBlock {
	DirectionalLight dirLight;
	PointLight       pointLight[MAX_POINT_LIGHTS];
	SpotLight        spotLight[MAX_SPOT_LIGHTS];
	int              pointLightCount;
	int              spotLightCount;
};

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 10) uniform sampler2D depthTexture;

layout(location = 0) out vec4 outputColor;

const float alphaTestThreshold = 0.1f;
const vec4 MaterialSpecularColor = vec4(0.1f, 0.1f, 0.1f, 1.0f); // TODO:
const vec4 MaterialDiffuseColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); // TODO:
const float specularIntensity = 0.1f;
const float ambientStrength = 0.1f;

vec3 calculateDirectionalLight(vec3 normal, vec3 lightDir, vec3 viewDir)
{
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float diffuseImpact = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = diffuseImpact * dirLight.color * dirLight.intensity;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0f), MaterialSpecularColor.w);
	vec3 specular = specularIntensity * spec * MaterialSpecularColor.rgb * dirLight.intensity;

	return diffuse + specular;
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragWorldPosition.xyz);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float diffuseImpact = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = diffuseImpact * light.color * light.intensity;

	float distance = length(light.position - fragWorldPosition.xyz);
	float attenuation = 1.0f / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
	diffuse *= attenuation;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0f), MaterialSpecularColor.w);
	vec3 specular = specularIntensity * spec * (MaterialSpecularColor.xyz * light.intensity) * attenuation;

	return diffuse + specular;
}

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir) 
{
	float innerCutoff = light.angles.x;
	float outerCutoff = light.angles.y;

	vec3 lightDir = normalize(light.position - fragWorldPosition.xyz);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float diffuseImpact = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = diffuseImpact * light.color * light.intensity;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0f), MaterialSpecularColor.w);
	vec3 specular = specularIntensity * spec * MaterialSpecularColor.xyz * light.intensity;

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = innerCutoff - outerCutoff;
	float intensity = clamp((theta - outerCutoff) / epsilon, 0.0f, 1.0f);
	diffuse *= intensity;
	specular *= intensity;

	float distance = length(light.position - fragWorldPosition.xyz);
	float attenuation = 1.0f / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
	diffuse *= attenuation;
	specular *= attenuation;

	return diffuse + specular;
}

// Проверка тени (PCF-like)
float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5f + 0.5f; // [-1,1] => [0,1]
	if (projCoords.z > 1.0f) return 0.0f;

	float currentDepth = projCoords.z;

	float bias = max(0.002f * (1.0f - dot(normal, lightDir)), 0.0005f);

	// Простой PCF (3x3)
	vec2 texelSize = 1.0f / vec2(textureSize(depthTexture, 0));
	float pcfShadow = 0.0f;
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			float pcfDepth = texture(depthTexture, projCoords.xy + vec2(x, y) * texelSize).r;
			pcfShadow += currentDepth > (pcfDepth + bias) ? 1.0f : 0.0f;
		}
	}
	pcfShadow /= 9.0f;
	return pcfShadow;
}

void main()
{
	vec4 albedo = texture(diffuseTexture, fragTexCoord);
	if (albedo.a <= alphaTestThreshold) discard;

	vec4 baseColor = albedo * MaterialDiffuseColor;

	vec3 normal = normalize(fragNormal);
	vec3 viewDir = normalize(fragCameraPosition - fragWorldPosition.xyz);

	vec3 ambient = ambientStrength * baseColor.rgb;
	
	vec3 lighting = ambient;
	
	// Directional Light
	{
		vec4 fragPosLightSpace = dirLight.lightSpaceMatrix * fragWorldPosition;

		const vec3 lightDir = normalize(-dirLight.direction);
		const float shadow = calculateShadow(fragPosLightSpace, normal, lightDir);
		lighting += (1.0f - shadow) * calculateDirectionalLight(normal, lightDir, viewDir);
	}

	// Point Light
	for (int i = 0; i < pointLightCount; ++i)
	{
		lighting += calculatePointLight(pointLight[i], normal, viewDir);
	}

	// Spot Light
	for (int i = 0; i < spotLightCount; ++i)
	{
		lighting += calculateSpotLight(spotLight[i], normal, viewDir);
	}

	// Final
	outputColor.rgb = baseColor.rgb * lighting;
	outputColor.a = baseColor.a;
}