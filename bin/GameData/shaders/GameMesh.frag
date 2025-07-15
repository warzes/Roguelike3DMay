#version 460 core

struct Light
{
	vec3  diffuseColor;
	vec3  direction;
	vec3  ambientLight;
	vec3  specularColor;
	float specularPower;
	vec3  position;
};

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoords;
layout(location = 2) in vec4 worldPosition;

layout(binding = 0) uniform sampler2D diffuseTex;

layout(binding = 1, std140) uniform SceneUniforms { 
	uniform float NumLight;
};

layout(binding = 0, std430) readonly buffer LightSSBO
{
	Light light[];
};

layout(location = 0) out vec4 OutFragColor;

void main()
{
	vec4 diffuseTexture = texture(diffuseTex, fragTexCoords);
	if (diffuseTexture.a < 0.2)
		discard;

	vec4 lightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	for(int i = 0; i < NumLight; i++)
	{
		float lightAngle = max(dot(normalize(fragNormal), normalize(light[i].position.xyz)), 0.0);
		lightColor += vec4((0.3 + 0.7 * lightAngle) * light[i].diffuseColor, 1.0);
	}
	lightColor = clamp(lightColor, 0.0f, 1.0f);

	OutFragColor = diffuseTexture * lightColor;
}