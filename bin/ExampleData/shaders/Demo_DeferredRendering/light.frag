#version 460 core

layout(binding = 0) uniform sampler2D gbufTex0;
layout(binding = 1) uniform sampler2D gbufTex1;
layout(binding = 2) uniform sampler2D gbufTex2;


struct Light
{
	vec3 position;
	vec3 color;
};

layout(std430, binding = 0) buffer LightBuffer 
{
	Light data[1024];
	int num_lights;
} lightBuffer;

struct fragment_info_t
{
	vec3 color;
	vec3 normal;
	float specular_power;
	vec3 ws_coord;
};

layout(location = 0) out vec4 outputColor;

void unpackGBuffer(ivec2 coord, out fragment_info_t fragment)
{
	fragment.color = texture(gbufTex0, coord).rgb;
	fragment.normal = texture(gbufTex1, coord).rgb;
	fragment.ws_coord = texture(gbufTex2, coord).rgb;
	fragment.specular_power = 60.0f;
}

vec4 light_fragment(fragment_info_t fragment)
{
	int i;
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);

	for (i = 0; i < lightBuffer.num_lights; i++)
	{
		Light light = lightBuffer.data[i];
		vec3 L = vec3(light.position.x, light.position.y, light.position.z) - fragment.ws_coord;
		float dist = length(L);
		L = normalize(L);
		vec3 N = normalize(fragment.normal);
		vec3 R = reflect(-L, N);
		float NdotR = max(0.0, dot(N, R));
		float NdotL = max(0.0, dot(N, L));

		float attenuation = clamp(1.0 - dist * dist / (5.0f * 5.0f), 0.0, 1.0);
		//float attenuation = 50.0 / (pow(dist, 2.0) + 1.0);

		vec3 diffuse_color  = 1.0 * vec3(light.color.x, light.color.y, light.color.z) * fragment.color * NdotL * attenuation;
		vec3 specular_color = vec3(1.0) /* * light[i].color */* pow(NdotR, fragment.specular_power) * attenuation;

		result += vec4(diffuse_color + specular_color, 0.0);
	}

	return result;
}

void main()
{
	fragment_info_t fragment;

	unpackGBuffer(ivec2(gl_FragCoord.xy), fragment);

	outputColor = light_fragment(fragment);
}