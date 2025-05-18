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

float getAttenuation(float lightRadius, float dist)
{
	return clamp(1.0 - dist * dist / (lightRadius * lightRadius), 0.0, 1.0);
//	float cutoff = 0.3;
//	float denom = dist / lightRadius + 1.0;
//	float attenuation = 1.0 / (denom * denom);
//
//	attenuation = (attenuation - cutoff) / (1 - cutoff);
//	attenuation = max(attenuation, 0.0);
//	return attenuation;
}

void main()
{
	// Determine which tile this pixel belongs to
	ivec2 loc = ivec2(gl_FragCoord.xy);
	ivec2 tileID = loc / ivec2(TILE_SIZE, TILE_SIZE);
	uint tileIndex = tileID.y * workgroupX + tileID.x;
    uint offset = tileIndex * MAX_LIGHTS_PER_TILE;

	// debug view
	if (doLightDebug>0)
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

	vec4 albedo4 = texture(texture_diffuse1, fragment_in.frag_uv);
	if (albedo4.a <= 0.2) discard;
	//vec3 albedo = pow(albedo4.rgb, vec3(2.2));
	vec3 albedo = albedo4.rgb;	
	float alpha = albedo4.a;

	vec3 normal = texture(texture_normal1, fragment_in.frag_uv).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 viewDirTS = normalize(fragment_in.ts_view_pos - fragment_in.ts_frag_pos);
	float specpower = 60.0f;

	vec3 result = vec3(0.0);
	for (uint i = 0; i < MAX_LIGHTS_PER_TILE; i++)
	{
		if (lights_indices[offset + i] != -1)
		{
			// TODO: возможно надо break чтобы не перебирать все
			int indices = lights_indices[offset + i];
			PointLight light = lights[indices]; // почему-то так работает эффективнее - дает больше фпс wtf

			vec3 lightPosTS = fragment_in.TBN * light.position;
			vec3 lightDirTS = lightPosTS - fragment_in.ts_frag_pos;
			float dist = length(lightDirTS);
			lightDirTS /= dist; // normalize
			
			float attenuation = getAttenuation(light.radius, dist);	
			if (attenuation == 0.0) continue;

			vec3 radiance = light.color * attenuation;

			float NdotL = max(0.0, dot(normal, lightDirTS));
			if (NdotL == 0.0) continue;

			// Диффузное освещение
			vec3 diffuse = albedo * NdotL * radiance;

			// Спекулярное освещение
			vec3 reflectDir = reflect(-lightDirTS, normal);
			float spec = pow(max(dot(viewDirTS, reflectDir), 0.0), specpower);
			vec3 specular = vec3(1.0) * spec * attenuation;

			result += diffuse + specular;
		}
	}
	
	outFragColor = vec4(result, alpha);
}