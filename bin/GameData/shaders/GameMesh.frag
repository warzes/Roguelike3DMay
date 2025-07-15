#version 460 core

// TODO: переделать DirectionalLight и PointLight в один Light

struct DirectionalLight
{
	vec3  position;
	vec3  color;

	vec3  ambient;
	vec3  diffuse;
	vec3  specular;

	float intensity;

	vec3  direction;
};

struct PointLight
{
	vec3  position;
	vec3  color;

	vec3  ambient;
	vec3  diffuse;
	vec3  specular;

	float intensity;

	float constant;
	float linear;
	float quadratic;
};

layout(location = 0) in vec3 FragPosition;
layout(location = 1) in vec3 FragNormal;
layout(location = 2) in vec2 FragTexCoords;
layout(location = 3) in vec3 TangentViewPos;
layout(location = 4) in vec3 TangentFragPos;

layout(binding = 0) uniform sampler2D diffuseTex;

layout(binding = 1, std140) uniform SceneUniforms { 
	uniform int NumDirectionalLight;
	uniform int NumPointLight;
	uniform vec3 cameraPos;
};

layout(binding = 0, std430) readonly buffer DirectionalLightSSBO
{
	DirectionalLight directionalLight[];
};

layout(binding = 1, std430) readonly buffer PointLightSSBO
{
	PointLight pointLight[];
};

layout(location = 0) out vec4 OutFragColor;

vec4 calculateAmbient(DirectionalLight light)
{
	vec4 ambient = vec4(1.0);

	vec4 lightAmbient = vec4(light.ambient, 1.0);
	vec4 lightColor = vec4(light.color, 1.0);

	//if(hasDiffuse)
	{
		vec4 textureDiffuse = texture(diffuseTex, FragTexCoords);
		ambient = lightAmbient * lightColor * textureDiffuse;
	}
	//else ambient = lightAmbient * lightColor * vec4(ourColor, 1.0);

	return ambient;
}

vec4 calculateAmbient(PointLight light)
{
	vec4 ambient = vec4(1.0);

	vec4 lightAmbient = vec4(light.ambient, 1.0);
	vec4 lightColor = vec4(light.color, 1.0);

	//if(hasDiffuse)
	{
		vec4 textureDiffuse = texture(diffuseTex, FragTexCoords);
		ambient = lightAmbient * lightColor * textureDiffuse;
	}
	//else ambient = lightAmbient * lightColor * vec4(ourColor, 1.0);

	return ambient;
}

vec4 calculateDiffuse(DirectionalLight light, vec3 normal, vec3 lightDir)
{
	vec4 diffuse = vec4(1.0);
	float diff = max(dot(normal, lightDir), 0.0);

	vec4 lightDiffuse = vec4(light.diffuse, 1.0);
	//vec4 materialDiffuse = vec4(material.diffuse, 1.0);
	vec4 materialDiffuse = vec4(1.0);

	//if(hasDiffuse)
	{
		vec4 textureDiffuse = texture(diffuseTex, FragTexCoords);
		diffuse = lightDiffuse * diff * textureDiffuse;
	}
	//else diffuse = lightDiffuse * (diff * materialDiffuse);

	return diffuse;
}

vec4 calculateDiffuse(PointLight light, vec3 normal, vec3 lightDir)
{
	vec4 diffuse = vec4(1.0);
	float diff = max(dot(normal, lightDir), 0.0);

	vec4 lightDiffuse = vec4(light.diffuse, 1.0);
	//vec4 materialDiffuse = vec4(material.diffuse, 1.0);
	vec4 materialDiffuse = vec4(1.0);

	//if(hasDiffuse)
	{
		vec4 textureDiffuse = texture(diffuseTex, FragTexCoords);
		diffuse = lightDiffuse * diff * textureDiffuse;
	}
	//else diffuse = lightDiffuse * (diff * materialDiffuse);

	return diffuse;
}

vec4 calculateSpecular(DirectionalLight light, vec3 normal, vec3 lightDir)
{
	vec4 specular = vec4(1.0);
	vec3 viewDir = normalize(cameraPos - FragPosition);

	//if(hasNormalMap) viewDir = normalize(TangentViewPos - TangentFragPos);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;

	vec4 lightSpecular = vec4(light.specular, 1.0);
	//vec4 materialSpecular = vec4(material.specular, 1.0);
	vec4 materialSpecular = vec4(0.0);

	//if(blinn)
	{
		vec3 halfwayDir = normalize(lightDir + viewDir);
		//spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
		spec = pow(max(dot(normal, halfwayDir), 0.0), 1.0);
	}
	//else spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	//if(hasSpecular)
	//{
	//	vec4 specularMapInfo = texture(materialMaps.specularMap, texCoord);
	//	specular = lightSpecular * spec * specularMapInfo;
	//}
	//else 
		specular = lightSpecular * (spec * materialSpecular);

	return specular;
}

vec4 calculateSpecular(PointLight light, vec3 normal, vec3 lightDir)
{
	vec4 specular = vec4(1.0);
	vec3 viewDir = normalize(cameraPos - FragPosition);

	//if(hasNormalMap) viewDir = normalize(TangentViewPos - TangentFragPos);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;

	vec4 lightSpecular = vec4(light.specular, 1.0);
	//vec4 materialSpecular = vec4(material.specular, 1.0);
	vec4 materialSpecular = vec4(0.0);

	//if(blinn)
	{
		vec3 halfwayDir = normalize(lightDir + viewDir);
		//spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
		spec = pow(max(dot(normal, halfwayDir), 0.0), 1.0);
	}
	//else spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	//if(hasSpecular)
	//{
	//	vec4 specularMapInfo = texture(materialMaps.specularMap, texCoord);
	//	specular = lightSpecular * spec * specularMapInfo;
	//}
	//else 
		specular = lightSpecular * (spec * materialSpecular);

	return specular;
}

vec4 calculateEmission()
{
	vec4 emission = vec4(0.0);
	//if(hasEmission) emission = texture(materialMaps.emissionMap, texCoord);
	//return emission * emissionStrength;
	return emission;
}

vec4 getDirectionalLightColor(DirectionalLight light)
{
	// Calculate ambient, diffuse and specular
	vec3 norm = normalize(FragNormal);

	vec3 lightDir = normalize(light.position - FragPosition);

//	if(hasNormalMap)
//	{
//		norm = texture(normalMap, FragTexCoords).rgb;
//	// transform normal vector to range [-1,1]
//		norm = normalize(norm * 2.0 - 1.0);  // this normal is in tangent space
//		lightDir = normalize(TangentLightPos - TangentFragPos);
//	}

	vec4 ambient = calculateAmbient(light);
	vec4 diffuse = calculateDiffuse(light, norm, lightDir);
	vec4 specular = calculateSpecular(light, norm, lightDir);

	// Calculate emission
	vec4 emission = calculateEmission();

	// Calculate shadow
	float shadow = 0.0;
	//if(shadowMapping) shadow = calculateShadow(FragPosLightSpace);

	vec4 lightColor = vec4(light.color, 1.0);
	return (ambient + (1.0 - shadow) * (diffuse + specular) + emission) * lightColor;
}

vec4 getPointLightColor(PointLight light)
{
	// Calculate ambient, diffuse and specular
	vec3 norm = normalize(FragNormal);

	vec3 lightDir = normalize(light.position - FragPosition);

//	if(hasNormalMap)
//	{
//		norm = texture(normalMap, FragTexCoords).rgb;
//	// transform normal vector to range [-1,1]
//		norm = normalize(norm * 2.0 - 1.0);  // this normal is in tangent space
//		lightDir = normalize(TangentLightPos - TangentFragPos);
//	}

	vec4 ambient = calculateAmbient(light);
	vec4 diffuse = calculateDiffuse(light, norm, lightDir);
	vec4 specular = calculateSpecular(light, norm, lightDir);

	// Calculate emission
	vec4 emission = calculateEmission();

	// Calculate attenuation
	float distance = length(light.position - FragPosition);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	// Calculate shadow
	float shadow = 0.0;
	//if(shadowMapping) shadow = calculateShadow(FragPosLightSpace);

	vec4 lightColor = vec4(light.color, 1.0);
	return (ambient + (1.0 - shadow) * (diffuse + specular) + emission) * lightColor;
}

void main()
{
	vec4 textureDiffuse = texture(diffuseTex, FragTexCoords);
	if (textureDiffuse.a < 0.2)
		discard;

	vec3 viewDir = normalize(TangentViewPos - TangentFragPos);

	vec4 lightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	for(int i = 0; i < NumDirectionalLight; i++)
	{
		lightColor += getDirectionalLightColor(directionalLight[i]);
		
	}
	for(int i = 0; i < NumPointLight; i++)
	{
		lightColor += getPointLightColor(pointLight[i]);
	}
	lightColor = clamp(lightColor, 0.0f, 1.0f);

	OutFragColor = lightColor;
	OutFragColor.a = textureDiffuse.a;
}