#pragma once

namespace
{
	const char* geometryVertexSource = R"(
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform float invertedNormals; // Should be -1.0 or 1.0

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 viewPos = view * model * vec4(aPos, 1.0);
	FragPos = viewPos.xyz;
	TexCoords = aTexCoords;

	mat3 normalMatrix = transpose(inverse(mat3(view * model)));
	Normal = normalMatrix * aNormal;

	gl_Position = projection * viewPos * invertedNormals;
}
)";

	const char* geometryFragmentSource = R"(
#version 460 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;

void main()
{
	gPosition = FragPos;
	gNormal = normalize(Normal);
	gAlbedo.rgb = vec3(1.0);
	//gAlbedo.rgb = texture(texture_diffuse1, TexCoords).rgb;
}
)";

	const char* ssaoVertexSource = R"(
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = vec4(aPos, 1.0);
}
)";

	const char* lightingFragmentSource = R"(
#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

struct Light
{
	vec3 Position;
	vec3 Color;
};
const int NR_LIGHTS = 200;
uniform Light lights[NR_LIGHTS];

uniform vec3 viewPos;
uniform float linear;
uniform float quadratic;

void main()
{
	// Retrieve data from G buffer
	vec3 FragPos = texture(gPosition, TexCoords).rgb;
	vec3 Normal = texture(gNormal, TexCoords).rgb;
	vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
	float AmbientOcclusion = texture(ssao, TexCoords).r;

	// Calculate lighting
	vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion);
	vec3 lighting = ambient;
	vec3 viewDir = normalize(viewPos - FragPos);

	for (int i = 0; i < NR_LIGHTS; ++i)
	{
		// Diffuse
		vec3 lightDir = normalize(lights[i].Position - FragPos);
		vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;

		// Specular
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
		vec3 specular = lights[i].Color * spec;

		// Attenuation
		float distance = length(lights[i].Position - FragPos);
		float attenuation = 1.0 / (1.0 + linear * distance + quadratic * distance * distance);
		diffuse *= attenuation;
		specular *= attenuation;

		lighting += diffuse + specular;
	}

	FragColor = vec4(lighting, 1.0);
}
)";

	const char* ssaoFragmentSource = R"(
#version 460 core

out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[256]; // Make this array long enough

// Parameters
uniform int kernelSize; // = 64;
uniform float radius; // = 0.5;
uniform float bias; // = 0.025;

uniform float screen_width;
uniform float screen_height;
uniform float noise_size;

uniform mat4 projection;

void main()
{
	vec2 noiseScale = vec2(screen_width / noise_size, screen_height / noise_size);

	// Get input for SSAO algorithm
	vec4 fragPos = texture(gPosition, TexCoords).xyzw;
	vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
	vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
	
	// Create TBN change-of-basis matrix: from tangent-space to view-space
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);
	
	// Iterate over the sample kernel and calculate occlusion factor
	float occlusion = 0.0;
	for (int i = 0; i < kernelSize; ++i)
	{
		// Get sample position
		vec3 samplePos = TBN * samples[i]; // From tangent to view-space
		samplePos = fragPos.xyz + samplePos * radius;

		// Project sample position (to sample texture) (to get position on screen/texture)
		vec4 offset = vec4(samplePos, 1.0);
		offset = projection * offset; // From view to clip-space
		offset.xyz /= offset.w; // Perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // Transform to range 0.0 - 1.0

		// Get sample depth
		float sampleDepth = texture(gPosition, offset.xy).z; // Get depth value of kernel sample

		// w = 1.0 if background, 0.0 if foreground
		float discardFactor = 1.0 - fragPos.w;

		// Range check & accumulate
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth)) * discardFactor;
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / kernelSize);

	FragColor = occlusion;
}
)";

	const char* blurFragmentSource = R"(
#version 460 core

out float FragColor;

in vec2 TexCoords;

uniform sampler2D ssaoInput;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
	float result = 0.0;
	for (int x = -2; x <= 2; ++x)
	{
		for (int y = -2; y <= 2; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(ssaoInput, TexCoords + offset).r;
		}
	}
	FragColor = result / 25.0;
}
)";
}