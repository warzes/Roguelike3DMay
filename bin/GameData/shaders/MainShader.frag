#version 460 core

#define MAX_NUM_LIGHT_SOURCES 5

#define PI 3.14159265

#define DIRECTIONAL_LIGHT 1
#define POINT_LIGHT 2

#define NEAR 0.1

// NOTE: display modes
#define HARD_SHADOWS 0
#define SOFT_SHADOWS 1
#define BLOCKER_SEARCH 2
#define PENUMBRA_ESTIMATE 3

struct LightSource
{
	vec3  diffuseColor;
	float diffusePower;
	vec3  specularColor;
	float specularPower;
	vec3  position;
	int   type;
	float size;
};

layout(location = 0) in vec2 vTexCoords;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vViewDir;
layout(location = 4) in vec3 vWorldPosition;
layout(location = 5) in vec3 vCameraPosition;
layout(location = 6) in vec3 vEyePosition;

layout(binding = 0) uniform sampler2D diffuseTex;
layout(binding = 1) uniform sampler2D specularTex;
layout(binding = 2) uniform sampler2D emissionTex;
layout(binding = 3) uniform sampler2D normalTex;
layout(binding = 4) uniform sampler2D depthTex;

layout(binding = 5) uniform sampler1D distribution0;
layout(binding = 6) uniform sampler1D distribution1;

layout(binding = 7) uniform sampler2D shadowMap0;
layout(binding = 8) uniform sampler2D shadowMap1;
layout(binding = 9) uniform sampler2D shadowMap2;
layout(binding = 10) uniform sampler2D shadowMap3;
layout(binding = 11) uniform sampler2D shadowMap4;

layout(binding = 12) uniform samplerCube shadowCubeMap0;
layout(binding = 13) uniform samplerCube shadowCubeMap1;
layout(binding = 14) uniform samplerCube shadowCubeMap2;
layout(binding = 15) uniform samplerCube shadowCubeMap3;
layout(binding = 16) uniform samplerCube shadowCubeMap4;

layout(location = 0) out vec4 OutFragColor;

layout(binding = 0, std140) uniform GlobalUniforms { 
	mat4 view;
	mat4 projection;
	vec3 eyePosition;
};

layout(binding = 2, std140) uniform MaterialUniforms { 
	vec4 diffuse;

	bool hasDiffuseTexture;
	bool hasSpecularTexture;
	bool hasEmissionTexture;
	bool hasNormalMapTexture;
	bool hasDepthMapTexture;

	bool noLighing;
};

layout(binding = 3, std140) uniform MainUniforms { 
	mat4 invView;
	mat4 lightProjection;

	mat4 shadowMapViewProjection0;
	mat4 shadowMapViewProjection1;
	mat4 shadowMapViewProjection2;
	mat4 shadowMapViewProjection3;
	mat4 shadowMapViewProjection4;
	mat4 shadowMapViewProjection5;
	mat4 shadowMapViewProjection6;
	mat4 shadowMapViewProjection7;
	float directionalLightShadowMapBias;
	float pointLightShadowMapBias;

	int  MaxNumLightSources;
};

//layout(binding = 4, std140) uniform ShadowUniforms { 
//	mat4 shadowMapViewProjection0;
//	mat4 shadowMapViewProjection1;
//	mat4 shadowMapViewProjection2;
//	mat4 shadowMapViewProjection3;
//	mat4 shadowMapViewProjection4;
//	mat4 shadowMapViewProjection5;
//	mat4 shadowMapViewProjection6;
//	mat4 shadowMapViewProjection7;
//	float directionalLightShadowMapBias;
//	float pointLightShadowMapBias;
//};

vec3 specularColor = vec3(1);
float specularity = 30;
vec3 ambientColor = vec3(0.1);
float frustumSize = 1;
uniform int numBlockerSearchSamples = 1;
uniform int numPCFSamples = 1;
uniform int displayMode = 0;
uniform int selectedLightSource = -1;

// глобальные переменные
vec4 DiffuseColor;

layout(binding = 0, std430) readonly buffer LightSSBO
{
	LightSource lightSources[];
};

vec3 BlinnPhong(
	vec3 materialDiffuseColor,
	vec3 materialSpecularColor,
	float materialSpecularity,
	vec3 lightDiffuseColor,
	float lightDiffusePower,
	vec3 lightSpecularColor,
	float lightSpecularPower,
	float NdotL,
	float distanceAttenuation)
{
	return materialDiffuseColor * lightDiffuseColor * lightDiffusePower * NdotL / distanceAttenuation +
		pow(NdotL, materialSpecularity) * materialSpecularColor *
		lightSpecularColor * lightSpecularPower / distanceAttenuation;
}

vec3 LightContribution(vec3 diffuseColor, int i)
{
	switch (lightSources[i].type)
	{
	case DIRECTIONAL_LIGHT:
//		// почему это? а может и должно быть для DIRECTIONAL_LIGHT
//		float NdotH = max(0, dot(vNormal, normalize(lightDir + vViewDir)));
//		return BlinnPhong(diffuseColor, specularColor, specularity, lightSources[i].diffuseColor, lightSources[i].diffusePower, lightSources[i].specularColor, lightSources[i].specularPower, NdotH, lightDistance2);
		return BlinnPhong(
				diffuseColor,
				specularColor,
				specularity,
				lightSources[i].diffuseColor,
				lightSources[i].diffusePower,
				lightSources[i].specularColor,
				lightSources[i].specularPower,
				max(0, dot(vNormal, normalize(vViewDir - lightSources[i].position))),
				1);

	case POINT_LIGHT:
		vec3 lightDir = lightSources[i].position - vWorldPosition;
		float lightDistance2 = length(lightDir);
		lightDir /= lightDistance2;
		lightDistance2 *= lightDistance2;

		return BlinnPhong(
				diffuseColor,
				specularColor,
				specularity,
				lightSources[i].diffuseColor,
				lightSources[i].diffusePower,
				lightSources[i].specularColor,
				lightSources[i].specularPower,
				max(0, dot(vNormal, normalize(lightDir + vViewDir))),
				lightDistance2);
	default:
		return vec3(1,0,0);
	}
}

bool IsLightEnabled(int i)
{
	return lightSources[i].type != 0;
}

float HardShadow(int i)
{
	return 1.0f;
}

void DisplayHardShadows()
{
	int enabledLights = 0;
	for (int i = 0; i < MaxNumLightSources; i++)
	{
		if (IsLightEnabled(i))
		{
			enabledLights++;

			OutFragColor.rgb += LightContribution(DiffuseColor.rgb, i) * HardShadow(i);
		}
	}

	if (enabledLights > 0)
	{
		OutFragColor.rgb /= enabledLights;
	}
}

void DisplaySoftShadows()
{
	OutFragColor = vec4(ambientColor, DiffuseColor.a);

OutFragColor.rgb = DiffuseColor.rgb;
}

void DisplayBlockerSearch()
{
	OutFragColor = vec4(ambientColor, DiffuseColor.a);

OutFragColor.rgb = DiffuseColor.rgb;
}

void DisplayPenumbraEstimate()
{
	OutFragColor = vec4(ambientColor, DiffuseColor.a);

OutFragColor.rgb = DiffuseColor.rgb;
}

void main()
{
	// определение diffuse, либо с текстуры, либо с материала
	DiffuseColor = diffuse;
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

	OutFragColor = vec4(ambientColor, DiffuseColor.a);

// TODO: отделить тень от света
	switch (displayMode)
	{
	case HARD_SHADOWS:
		DisplayHardShadows();
		break;
	case SOFT_SHADOWS:
		DisplaySoftShadows();
		break;
	case BLOCKER_SEARCH:
		DisplayBlockerSearch();
		break;
	case PENUMBRA_ESTIMATE:
		DisplayPenumbraEstimate();
		break;
	default:
		OutFragColor = vec4(1,0,0,1);
	}
}