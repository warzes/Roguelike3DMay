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
layout(location = 3) in float MaxNumLight;

layout(binding = 0) uniform sampler2D diffuseTex;

layout(binding = 2, std430) readonly buffer LightSSBO
{
	Light light[];
};

//vec3 lightPos = vec3(4.0, 5.0, -3.0);
//vec3 lightColor = vec3(1.0, 1.0, 1.0);

layout(location = 0) out vec4 OutFragColor;

void main()
{
	vec4 diffuseTexture = texture(diffuseTex, fragTexCoords);
	if (diffuseTexture.a < 0.2)
		discard;

	vec4 lightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	for(int i = 0; i < MaxNumLight; i++)
	{
		float lightAngle = max(dot(normalize(fragNormal), normalize(light[i].position.xyz)), 0.0);
		lightColor += vec4((0.3 + 0.7 * lightAngle) * light[i].diffuseColor, 1.0);

		//vec3 lightPos = normalize(light[i].position.xyz - worldPosition.xyz);
		// Calculate the different amounts of light on this pixel based on the positions of the lights.
		//float lightIntensity = clamp(dot(fragNormal, lightPos), 0.0f, 1.0f);
		// Determine the diffuse color amount of each of the four lights.
		//lightColor += light[i].diffuseColor * lightIntensity;
	}
	lightColor = clamp(lightColor, 0.0f, 1.0f);

	//OutFragColor = diffuseTexture * vec4((0.3 + 0.7 * lightAngle) * lightColor, 1.0);
	OutFragColor = diffuseTexture * lightColor;
}