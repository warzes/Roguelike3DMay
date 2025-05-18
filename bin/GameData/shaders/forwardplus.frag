#version 460 core

#define MAX_LIGHTS_PER_TILE 128
#define TILE_SIZE 16

in VERTEX_OUT{
	vec2 frag_uv;
	mat3 TBN;
	vec3 ts_frag_pos;
	vec3 ts_view_pos;
} fragment_in;

struct PointLight {
	vec3 position;
	float radius;
	vec3 color;
	float intensity;
};

layout(std430, binding = 0) readonly buffer LightData {
	PointLight lights[];
};

layout(std430, binding = 1) readonly buffer VisibleLightsIndices {
	int lights_indices[];
};

uniform int doLightDebug;
uniform int workgroupX; // numberOfTilesX

layout(binding = 0) uniform sampler2D texture_diffuse1;
layout(binding = 1) uniform sampler2D texture_normal1;

layout(location = 0) out vec4 outFragColor;

void main()
{
	// Determine which tile this pixel belongs to
	ivec2 loc = ivec2(gl_FragCoord.xy);
	ivec2 tileID = loc / ivec2(TILE_SIZE, TILE_SIZE);
	uint index = tileID.y * workgroupX + tileID.x;
    uint offset = index * MAX_LIGHTS_PER_TILE;

	vec4 base_diffuse = texture(texture_diffuse1, fragment_in.frag_uv);
	if (base_diffuse.a <= 0.2) discard;

	if (doLightDebug==1)
	{
		uint count = 0;
		for (uint i = 0; i < MAX_LIGHTS_PER_TILE; i++)
		{
			if (lights_indices[offset + i] != -1 ) {
				count++;
			}
		}
		float shade = float(count) / float(MAX_LIGHTS_PER_TILE * 2); 
		outFragColor = vec4(shade);
		return;
	}

	vec3 normal = normalize((texture(texture_normal1, fragment_in.frag_uv).rgb * 2.0 - 1.0));
	float specpower = 60.0f;

	vec3 result = vec3(0.0);

	for (uint i = 0; i < MAX_LIGHTS_PER_TILE; i++)
	{
		int idx = lights_indices[offset + i];
		if (idx == -1) continue; // это правильно? или break

		PointLight light = lights[idx];

		vec3 lightPosTS = fragment_in.TBN * light.position;
		vec3 lightDirTS = lightPosTS - fragment_in.ts_frag_pos;
		float dist = length(lightDirTS);
		float attenuation = clamp(1.0 - dist * dist / (light.radius * light.radius), 0.0, 1.0);
		if (attenuation == 0.0)
			continue;

		lightDirTS /= dist; // normalize

		float NdotL = max(0.0, dot(normal, lightDirTS));
		if (NdotL == 0.0)
			continue;

		// Диффузное освещение
		vec3 diffuse = light.color * base_diffuse.rgb * NdotL * attenuation;

		// Спекулярное освещение
		vec3 viewDirTS = normalize(fragment_in.ts_view_pos - fragment_in.ts_frag_pos);
		vec3 reflectDir = reflect(-lightDirTS, normal);
		float spec = pow(max(dot(viewDirTS, reflectDir), 0.0), specpower);
		vec3 specular = vec3(1.0) * spec * attenuation;

		result += diffuse + specular;
	}
	
	outFragColor =vec4(result, 1.0);
}