#version 460 core

#define MAX_POINT_LIGHT_PER_TILE 2049
const int TILE_SIZE = 16;

in VERTEX_OUT{
	vec2 frag_uv;
	vec3 fragNormal;
	vec3 fragPos;
} fragment_in;

struct PointLight {
	vec3 position;
	float radius;
	vec3 color;
};

layout(std430, binding = 0) readonly buffer LightData {
	PointLight pointLights[];
};

struct LightVisibility {
	uint count;
	uint lightIndices[MAX_POINT_LIGHT_PER_TILE];
};

layout(std430, binding = 1) readonly buffer TileLightVisibilities {
	LightVisibility light_visiblities[];
};

//layout(stb430, binding = 0) uniform MaterialUbo
//{
//	int hasAlbedoMap;
//	int hasNormalMap;
//} material;

uniform ivec2 viewportSize;
uniform ivec2 tileNums;		// tileNums.x = workgroupX; tileNums.y = workgroupY
uniform int debugViewIndex;

uniform vec3 viewPosition;

layout(binding = 0) uniform sampler2D texture_diffuse1;
layout(binding = 1) uniform sampler2D texture_normal1;

layout(location = 0) out vec4 outFragColor;

layout(early_fragment_tests) in; // for early depth test

vec3 applyNormalMap(vec3 geomnor, vec3 normap)
{
	normap = normap * 2.0 - 1.0;
	vec3 up = normalize(vec3(0.001, 1, 0.001));
	vec3 surftan = normalize(cross(geomnor, up));
	vec3 surfbinor = cross(geomnor, surftan);
	return normalize(normap.y * surftan + normap.x * surfbinor + normap.z * geomnor);
}

void main()
{
	vec3 diffuse;
	float alphaDiffuse;
	// if (material.hasAlbedoMap > 0)
	{
		vec4 diffuse4 = texture(texture_diffuse1, fragment_in.frag_uv);
		if (diffuse4.a <= 0.2) discard;
		diffuse = diffuse4.rgb;
		alphaDiffuse = diffuse4.a;
	}
	// else
	//{
	//	diffuse = vec3(1.0);
	//	alphaDiffuse = 1.0f;
	//}

	vec3 normal;
	// if (material.hasNormalMap > 0)
	{
		normal = applyNormalMap(fragment_in.fragNormal, texture(texture_normal1, fragment_in.frag_uv).rgb);
	}
	// else
	//{
	//	normal = fragment_in.fragNormal;
	//}

	ivec2 tileID = ivec2(gl_FragCoord.xy / TILE_SIZE);
	uint tileIndex = tileID.y * tileNums.x + tileID.x;

	// debug view
	if (debugViewIndex > 1)
	{
		if (debugViewIndex == 2)
		{
			//heat map debug view
			float intensity = float(light_visiblities[tileIndex].count) / 64;
			outFragColor = vec4(vec3(intensity), 1.0) ; //light culling debug
			//out_color = vec4(vec3(intensity * 0.62, intensity * 0.13, intensity * 0.94), 1.0) ; //light culling debug
			//float minimum = 0.0;
			//float maximum = 1.0;
			//float ratio = 2 * (intensity - minimum) / (maximum - minimum);
			//float b = max(0, 1 - ratio);
			//float r = max(0, ratio - 1);
			//float g = max(0, 1.0 - b - r);
			//out_color = vec4(vec3(r,g,b), 1.0);
		}
		else if (debugViewIndex == 3)
		{
			// depth debug view
			//float preDepth = texture(depth_sampler, (gl_FragCoord.xy/push_constants.viewport_size) ).x;
			//out_color = vec4(vec3( pre_depth ),1.0);
		}
		else if (debugViewIndex == 4)
		{
			// normal debug view
			outFragColor = vec4(abs(normal), 1.0);
		}
		return;
	}

	vec3 illuminance = vec3(0.0);
	uint tileLightNum = light_visiblities[tileIndex].count;
	for (int i = 0; i < tileLightNum; i++)
	{
		PointLight light = pointLights[light_visiblities[tileIndex].lightIndices[i]];
		vec3 lightDir = normalize(light.position - fragment_in.fragPos);
		float lambertian = max(dot(lightDir, normal), 0.0);

		if (lambertian > 0.0)
		{
			float lightDistance = distance(light.position, fragment_in.fragPos);
			if (lightDistance > light.radius) continue;

			vec3 viewDir = normalize(viewPosition - fragment_in.fragPos);
			vec3 halfDir = normalize(lightDir + viewDir);
			float specAngle = max(dot(halfDir, normal), 0.0);
			float specular = pow(specAngle, 32.0); // TODO: spec color & power in gbuffer?

			float att = clamp(1.0 - lightDistance * lightDistance / (light.radius * light.radius), 0.0, 1.0);
			illuminance += light.color * att * (lambertian * diffuse + specular);
		}
	}

	// heat map with render debug view
	if (debugViewIndex == 1)
	{
		float intensity = float(light_visiblities[tileIndex].count) / (64 / 2.0);
		outFragColor = vec4(vec3(intensity, intensity * 0.5, intensity * 0.5) + illuminance * 0.25, 1.0) ; //light culling debug
		return;
	}

	outFragColor = vec4(illuminance, alphaDiffuse);
}