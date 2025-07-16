#version 460 core

// TODO: текстуры передавать вектором, а не высчитывать каждый раз.

struct Light
{
	vec3  position;
	vec3  color;

	vec3  ambient;
	vec3  diffuse;
	vec3  specular;
	float intensity;

	vec3  direction; // Directional Light
	float type;

	// Point Light
	float constant;
	float linear;
	float quadratic;
};

layout(location = 0) in vec3 FragPosition;
layout(location = 1) in vec3 FragColor;
layout(location = 2) in vec3 FragNormal;
layout(location = 3) in vec2 FragTexCoords;
//layout(location = ) in vec4 FragPosLightSpace;
//layout(location = ) in vec3 TangentLightPos;
layout(location = 4) in vec3 TangentViewPos;
layout(location = 5) in vec3 TangentFragPos;

layout(binding = 0) uniform sampler2D diffuseTex;
layout(binding = 1) uniform sampler2D specularTex;
layout(binding = 2) uniform sampler2D emissionTex;
layout(binding = 3) uniform sampler2D normalTex;
layout(binding = 4) uniform sampler2D depthTex;

//uniform bool shadowMapping;
//uniform sampler2D shadowMap;
//uniform vec3 lightPos;

layout(binding = 1, std140) uniform SceneUniforms { 
	uniform vec3 CameraPos;
	uniform int NumLight;
};
layout(binding = 3, std140) uniform MaterialUniforms { 
	uniform vec3 MatDiffuse;
	uniform float pag0;
	uniform vec3 MatSpecular;
	uniform float MatShininess;
	
	uniform bool hasDiffuse;
	uniform bool hasSpecular;
	uniform bool hasEmission;
	uniform bool hasNormalMap;
	uniform bool hasDepthMap;
	uniform float emissionStrength;
	uniform bool blinn;
	uniform float heightScale;
};

layout(binding = 0, std430) readonly buffer LightSSBO
{
	Light light[];
};

layout(location = 0) out vec4 OutFragColor;

vec2 texCoord = FragTexCoords;

vec4 calculateAmbient(Light light)
{
	vec4 ambient = vec4(1.0);

	vec4 lightAmbient = vec4(light.ambient, 1.0);
	vec4 lightColor = vec4(light.color, 1.0);

	if(hasDiffuse)
	{
		vec4 textureDiffuse = texture(diffuseTex, texCoord);
		ambient = lightAmbient * lightColor * textureDiffuse;
	}
	else 
		ambient = lightAmbient * lightColor * vec4(FragColor, 1.0);

	return ambient;
}

vec4 calculateDiffuse(Light light, vec3 normal, vec3 lightDir)
{
	vec4 diffuse = vec4(1.0);
	float diff = max(dot(normal, lightDir), 0.0);

	vec4 lightDiffuse = vec4(light.diffuse, 1.0);

	if(hasDiffuse)
	{
		vec4 textureDiffuse = texture(diffuseTex, texCoord);
		diffuse = lightDiffuse * diff * textureDiffuse;
	}
	else 
		diffuse = lightDiffuse * (diff * vec4(MatDiffuse, 1.0));

	return diffuse;
}

vec4 calculateSpecular(Light light, vec3 normal, vec3 lightDir)
{
	vec4 specular = vec4(1.0);
	vec3 viewDir = normalize(CameraPos - FragPosition);

	if(hasNormalMap) viewDir = normalize(TangentViewPos - TangentFragPos);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;

	vec4 lightSpecular = vec4(light.specular, 1.0);
	vec4 materialSpecular = vec4(MatSpecular, 1.0);

	if(blinn)
	{
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(normal, halfwayDir), 0.0), MatShininess);
	}
	else
		spec = pow(max(dot(viewDir, reflectDir), 0.0), MatShininess);

	if(hasSpecular)
	{
		vec4 specularMapInfo = texture(specularTex, texCoord);
		specular = lightSpecular * spec * specularMapInfo;
	}
	else 
		specular = lightSpecular * (spec * materialSpecular);

	return specular;
}

vec4 calculateEmission()
{
	vec4 emission = vec4(0.0);
	if(hasEmission) emission = texture(emissionTex, texCoord);
	return emission * emissionStrength;
}

//float calculateShadow(vec4 fragPosLightSpace)
//{
//	// perform perspective divide
//	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//	// transform to [0,1] range
//	projCoords = projCoords * 0.5 + 0.5;
//	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
//	float closestDepth = texture(shadowMap, projCoords.xy).r; 
//	// get depth of current fragment from light's perspective
//	float currentDepth = projCoords.z;
//	// calculate bias (based on depth map resolution and slope)
//	vec3 normal = normalize(Normal);
//	vec3 lightDir = normalize(lightPos - FragPos);
//	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
//	// check whether current frag pos is in shadow
//	// float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
//	// PCF
//	float shadow = 0.0;
//	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
//	for(int x = -1; x <= 1; ++x) {
//		for(int y = -1; y <= 1; ++y) {
//			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
//			shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
//		}
//	}
//	shadow /= 9.0;
//   
//	// keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
//	if(projCoords.z > 1.0)
//	shadow = 0.0;
//
//	return shadow;
//}

vec4 getLightColor(vec4 emission, Light light)
{
	// Calculate ambient, diffuse and specular
	vec3 norm = normalize(FragNormal);

	vec3 lightDir = normalize(light.position - FragPosition);

	if(hasNormalMap)
	{
		norm = texture(normalTex, texCoord).rgb;
		// transform normal vector to range [-1,1]
		norm = normalize(norm * 2.0 - 1.0);  // this normal is in tangent space
		//lightDir = normalize(TangentLightPos - TangentFragPos);
	}

	vec4 ambient = calculateAmbient(light);
	vec4 diffuse = calculateDiffuse(light, norm, lightDir);
	vec4 specular = calculateSpecular(light, norm, lightDir);

	// Calculate attenuation
	if(int(light.type) == 0) // pointLight
	{
		float distance = length(light.position - FragPosition);

		float attenuation = 1.0 / (
				light.constant + 
				light.linear * distance + 
				light.quadratic * distance * distance
			);
		ambient  *= attenuation;
		diffuse  *= attenuation;
		specular *= attenuation;
	}

	// Calculate shadow
	float shadow = 0.0;
//	//if(shadowMapping) shadow = calculateShadow(FragPosLightSpace);

	vec4 lightColor = vec4(light.color, 1.0);
	return (ambient + (1.0 - shadow) * (diffuse + specular) + emission) * lightColor;
}

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir)
{
	if(!hasDepthMap)
		return texCoords;

	// number of depth layers
	const float minLayers = 8;
	const float maxLayers = 32;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));

	// calculate the size of each layer
	float layerDepth = 1.0 / numLayers;

	// depth of current layer
	float currentLayerDepth = 0.0;

	// the amount to shift the texture coordinates per layer (from vector P)
	vec2 P = viewDir.xy / viewDir.z * heightScale;
	vec2 deltaTexCoords = P / numLayers;

	// get initial values
	vec2  currentTexCoords     = texCoords;
	float currentDepthMapValue = texture(depthTex, currentTexCoords).r;

	while(currentLayerDepth < currentDepthMapValue)
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;

		// get depthmap value at current texture coordinates
		currentDepthMapValue = texture(depthTex, currentTexCoords).r;

		// get depth of next layer
		currentLayerDepth += layerDepth;
	}

	// get texture coordinates before collision (reverse operations)
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for linear interpolation
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(depthTex, prevTexCoords).r - currentLayerDepth + layerDepth;

	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;
}

void main()
{
	if(hasDiffuse)
	{
		vec4 textureDiffuse = texture(diffuseTex, FragTexCoords);
		if (textureDiffuse.a < 0.2)
			discard;
	}

	vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
	texCoord = parallaxMapping(texCoord, viewDir);

	// Calculate emission
	vec4 emission = calculateEmission();

	vec4 lightColor = vec4(0.0);
	for(int i = 0; i < NumLight; i++)
	{
		lightColor += getLightColor(emission, light[i]);
	}
	lightColor = clamp(lightColor, 0.0f, 1.0f);

	if(hasDiffuse)
	{
		vec4 textureDiffuse = texture(diffuseTex, texCoord);
		lightColor.a = textureDiffuse.a;
	}

	OutFragColor = lightColor;
}