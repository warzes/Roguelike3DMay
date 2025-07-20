#version 460 core

#define PI 3.14159265

layout(location = 0) in vec3 FragPosition;
layout(location = 1) in vec3 FragColor;
layout(location = 2) in vec3 FragNormal;
layout(location = 3) in vec2 FragTexCoords;
layout(location = 4) in vec3 vViewDir;

layout(location = 5) in vec3 vCameraPosition;

layout(binding = 0) uniform sampler2D diffuseTex;
layout(binding = 1) uniform sampler2D specularTex;
layout(binding = 2) uniform sampler2D emissionTex;
layout(binding = 3) uniform sampler2D normalTex;
layout(binding = 4) uniform sampler2D depthTex;

layout(binding = 5) uniform sampler2D shadowMapTex;

layout(location = 0) out vec4 OutFragColor;

layout(binding = 2, std140) uniform MaterialUniforms { 
	uniform bool hasDiffuseTexture;
	uniform bool hasSpecularTexture;
	uniform bool hasEmissionTexture;
	uniform bool hasNormalMapTexture;
	uniform bool hasDepthMapTexture;

	uniform bool noLighing;
};

// BlinnPhong data
vec3 specularColor = vec3(1, 1, 1);
float specularity = 30;
vec3 lightDiffuseColor = vec3(1, 1, 1);
float lightDiffusePower = 1.0f;
vec3 lightSpecularColor = vec3(1, 1, 1);
float lightSpecularPower = 1.0f;
vec3 lightPosition = vec3(2.0f, 2.0f, 1.0f);

vec3 GetBlinnPhong(vec4 diffuse, vec3 lightDir, float lightDistance2)
{
	float NdotH = max(0, dot(FragNormal, normalize(lightDir + vViewDir)));

	return NdotH * diffuse.rgb *
					lightDiffuseColor * lightDiffusePower / lightDistance2 +
				pow(NdotH, specularity) * specularColor *
					lightSpecularColor * lightSpecularPower / lightDistance2;
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
	vec4 diffuse = vec4(0.8, 0.3, 1.0, 1.0);
	if (hasDiffuseTexture)
	{
		diffuse = texture(diffuseTex, FragTexCoords);
		if (diffuse.a < 0.2)
			discard;
	}

	if (noLighing)
	{
		OutFragColor = diffuse;
		return;
	}

	vec3 lightDir = lightPosition - FragPosition;
	float lightDistance2 = length(lightDir);
	lightDir /= lightDistance2;
	lightDistance2 *= lightDistance2;

	//OutFragColor = vec4(GetBlinnPhong(diffuse, lightDir, lightDistance2), 1.0);
	OutFragColor = vec4(CookTorrance(diffuse,
		specularColor2,
		FragNormal,
		lightDir,
		vViewDir,
		lightColor), 1.0);
}