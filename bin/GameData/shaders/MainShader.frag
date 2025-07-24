#version 460 core

#define MAX_NUM_LIGHT_SOURCES 8

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
layout(binding = 12) uniform sampler2D shadowMap5;
layout(binding = 13) uniform sampler2D shadowMap6;
layout(binding = 14) uniform sampler2D shadowMap7;

layout(binding = 15) uniform samplerCube shadowCubeMap0;
layout(binding = 16) uniform samplerCube shadowCubeMap1;
layout(binding = 17) uniform samplerCube shadowCubeMap2;
layout(binding = 18) uniform samplerCube shadowCubeMap3;
layout(binding = 19) uniform samplerCube shadowCubeMap4;
layout(binding = 20) uniform samplerCube shadowCubeMap5;
layout(binding = 21) uniform samplerCube shadowCubeMap6;
layout(binding = 22) uniform samplerCube shadowCubeMap7;

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

float Depth(vec3 pos)
{
    vec3 absPos = abs(pos);
	float z = -max(absPos.x, max(absPos.y, absPos.z));
	vec4 clip = lightProjection * vec4(0.0, 0.0, z, 1.0);
	return (clip.z / clip.w) * 0.5 + 0.5;
}

vec3 ShadowCoords(mat4 shadowMapViewProjection)
{
	vec4 projectedCoords = shadowMapViewProjection * vec4(vWorldPosition, 1);
	vec3 shadowCoords = projectedCoords.xyz / projectedCoords.w;
	shadowCoords = shadowCoords * 0.5 + 0.5;
	return shadowCoords;
}

float ShadowMapping_DirectionalLight(vec3 shadowCoords, sampler2D shadowMap, float uvLightSize)
{
	float z = texture(shadowMap, shadowCoords.xy).x;
	return (z < (shadowCoords.z - directionalLightShadowMapBias)) ? 0 : 1;
}

float ShadowMapping_PointLight(vec3 lightPosition, samplerCube shadowCubeMap, float uvLightSize)
{
	mat4 lightView = mat4(1,0,0,0, 
		0,1,0,0, 
		0,0,1,0, 
		-lightPosition.x,-lightPosition.y,-lightPosition.z, 1);
	vec3 positionLightSpace = (lightView * invView * vec4(vCameraPosition, 1)).xyz;
	float receiverDistance = Depth(positionLightSpace);
	float z = texture(shadowCubeMap, positionLightSpace).r;
	return (z < (receiverDistance - pointLightShadowMapBias)) ? 0 : 1;
}

float HardShadow(int i)
{
	if (i == 0)
	{
		if (IsLightEnabled(0))
		{
			switch (lightSources[0].type)
			{
			case DIRECTIONAL_LIGHT:
				return ShadowMapping_DirectionalLight(ShadowCoords(shadowMapViewProjection0), shadowMap0, lightSources[0].size / frustumSize);
			case POINT_LIGHT:
				return ShadowMapping_PointLight(lightSources[0].position, shadowCubeMap0, lightSources[0].size / frustumSize);
			}
		}
		return 0;
	}
	else if (i == 1)
	{
		if (IsLightEnabled(1))
		{
			switch (lightSources[1].type)
			{
			case DIRECTIONAL_LIGHT:
				return ShadowMapping_DirectionalLight(ShadowCoords(shadowMapViewProjection1), shadowMap1, lightSources[1].size / frustumSize);
			case POINT_LIGHT:
				return ShadowMapping_PointLight(lightSources[1].position, shadowCubeMap1, lightSources[1].size / frustumSize);
			}
		}
		return 0;
	}
	else if (i == 2)
	{
		if (IsLightEnabled(2))
		{
			switch (lightSources[2].type)
			{
			case DIRECTIONAL_LIGHT:
				return ShadowMapping_DirectionalLight(ShadowCoords(shadowMapViewProjection2), shadowMap2, lightSources[2].size / frustumSize);
			case POINT_LIGHT:
				return ShadowMapping_PointLight(lightSources[2].position, shadowCubeMap2, lightSources[2].size / frustumSize);
			}
		}
		return 0;
	}
	else if (i == 3)
	{
		if (IsLightEnabled(3))
		{
			switch (lightSources[3].type)
			{
			case DIRECTIONAL_LIGHT:
				return ShadowMapping_DirectionalLight(ShadowCoords(shadowMapViewProjection3), shadowMap3, lightSources[3].size / frustumSize);
			case POINT_LIGHT:
				return ShadowMapping_PointLight(lightSources[3].position, shadowCubeMap3, lightSources[3].size / frustumSize);
			}
		}
		return 0;
	}
	else if (i == 4)
	{
		if (IsLightEnabled(4))
		{
			switch (lightSources[4].type)
			{
			case DIRECTIONAL_LIGHT:
				return ShadowMapping_DirectionalLight(ShadowCoords(shadowMapViewProjection4), shadowMap4, lightSources[4].size / frustumSize);
			case POINT_LIGHT:
				return ShadowMapping_PointLight(lightSources[4].position, shadowCubeMap4, lightSources[4].size / frustumSize);
			}
		}
		return 0;
	}
	else if (i == 5)
	{
		if (IsLightEnabled(5))
		{
			switch (lightSources[5].type)
			{
			case DIRECTIONAL_LIGHT:
				return ShadowMapping_DirectionalLight(ShadowCoords(shadowMapViewProjection5), shadowMap5, lightSources[5].size / frustumSize);
			case POINT_LIGHT:
				return ShadowMapping_PointLight(lightSources[5].position, shadowCubeMap5, lightSources[5].size / frustumSize);
			}
		}
		return 0;
	}
	else if (i == 6)
	{
		if (IsLightEnabled(6))
		{
			switch (lightSources[6].type)
			{
			case DIRECTIONAL_LIGHT:
				return ShadowMapping_DirectionalLight(ShadowCoords(shadowMapViewProjection6), shadowMap6, lightSources[6].size / frustumSize);
			case POINT_LIGHT:
				return ShadowMapping_PointLight(lightSources[6].position, shadowCubeMap6, lightSources[6].size / frustumSize);
			}
		}
		return 0;
	}
	else if (i == 7)
	{
		if (IsLightEnabled(7))
		{
			switch (lightSources[7].type)
			{
			case DIRECTIONAL_LIGHT:
				return ShadowMapping_DirectionalLight(ShadowCoords(shadowMapViewProjection7), shadowMap7, lightSources[7].size / frustumSize);
			case POINT_LIGHT:
				return ShadowMapping_PointLight(lightSources[7].position, shadowCubeMap7, lightSources[7].size / frustumSize);
			}
		}
		return 0;
	}
	else
		return 0;
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