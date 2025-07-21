#version 460 core

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

layout(binding = 5) uniform sampler2D shadowMap0;
layout(binding = 6) uniform sampler2D shadowMap1;
layout(binding = 7) uniform sampler2D shadowMap2;
layout(binding = 8) uniform sampler2D shadowMap3;
layout(binding = 9) uniform sampler2D shadowMap4;
layout(binding = 10) uniform sampler2D shadowMap5;
layout(binding = 11) uniform sampler2D shadowMap6;
layout(binding = 12) uniform sampler2D shadowMap7;

layout(binding = 13) uniform samplerCube shadowCubeMap0;
layout(binding = 14) uniform samplerCube shadowCubeMap1;
layout(binding = 15) uniform samplerCube shadowCubeMap2;
layout(binding = 16) uniform samplerCube shadowCubeMap3;
layout(binding = 17) uniform samplerCube shadowCubeMap4;
layout(binding = 18) uniform samplerCube shadowCubeMap5;
layout(binding = 19) uniform samplerCube shadowCubeMap6;
layout(binding = 20) uniform samplerCube shadowCubeMap7;

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

	int  numLight;
};

vec3 specularColor = vec3(1);
float specularity = 30;
vec3 ambientColor = vec3(0.1);

layout(binding = 0, std430) readonly buffer LightSSBO
{
	LightSource lightSources[];
};

////////////////////////////////////////////////////////
// предыдущий код
////////////////////////////////////////////////////////
vec3 GetBlinnPhong(LightSource light, vec4 diffuse, vec3 lightDir, float lightDistance2)
{
	float NdotH = max(0, dot(vNormal, normalize(lightDir + vViewDir)));

	return NdotH * diffuse.rgb *
					light.diffuseColor * light.diffusePower / lightDistance2 +
				pow(NdotH, specularity) * specularColor *
					light.specularColor * light.specularPower / lightDistance2;
}

// CookTorrance data
vec3 specularColor2 = vec3(1, 1, 1);
float F0 = 0.8;
float roughness = 0.1;
float k = 0.2;
vec3 lightColor = vec3(1, 1, 0.8);

vec3 CookTorrance(vec4 materialDiffuseColor,
	vec3 materialSpecularColor,
	vec3 normal,
	vec3 lightDir,
	vec3 viewDir,
	vec3 lightColor)
{
	float NdotL = max(0, dot(normal, lightDir));
	float Rs = 0.0;
	if (NdotL > 0) 
	{
		vec3 H = normalize(lightDir + viewDir);
		float NdotH = max(0, dot(normal, H));
		float NdotV = max(0, dot(normal, viewDir));
		float VdotH = max(0, dot(lightDir, H));

		// Fresnel reflectance
		float F = pow(1.0 - VdotH, 5.0);
		F *= (1.0 - F0);
		F += F0;

		// Microfacet distribution by Beckmann
		float m_squared = roughness * roughness;
		float r1 = 1.0 / (4.0 * m_squared * pow(NdotH, 4.0));
		float r2 = (NdotH * NdotH - 1.0) / (m_squared * NdotH * NdotH);
		float D = r1 * exp(r2);

		// Geometric shadowing
		float two_NdotH = 2.0 * NdotH;
		float g1 = (two_NdotH * NdotV) / VdotH;
		float g2 = (two_NdotH * NdotL) / VdotH;
		float G = min(1.0, min(g1, g2));

		Rs = (F * D * G) / (PI * NdotL * NdotV);
	}
	return materialDiffuseColor.rgb * lightColor * NdotL + lightColor * materialSpecularColor * Rs;
}

void main()
{
	vec4 diffuseColor = diffuse;
	if (hasDiffuseTexture)
	{
		diffuseColor = texture(diffuseTex, vTexCoords);
		if (diffuseColor.a < 0.2)
			discard;
	}

	if (noLighing)
	{
		OutFragColor = diffuseColor;
		return;
	}

	vec3 lightColor = vec3(ambientColor);
	for (int i = 0; i < numLight; i++)
	{
		LightSource light = lightSources[i];

		vec3 lightDir = light.position - vWorldPosition;
		float lightDistance2 = length(lightDir);
		lightDir /= lightDistance2;
		lightDistance2 *= lightDistance2;

		lightColor += GetBlinnPhong(light, diffuseColor, lightDir, lightDistance2);
//	CookTorrance(diffuseColor,
//		specularColor2,
//		FragNormal,
//		lightDir,
//		vViewDir,
//		lightColor), 1.0);
	}

	lightColor.rgb /= numLight; // усреднение

	OutFragColor = vec4(lightColor, diffuseColor.a);
}