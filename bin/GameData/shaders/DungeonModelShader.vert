#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoords;
layout (location = 4) in vec3 aTangent;

layout(location = 0) out vec2 vTexCoords;
layout(location = 1) out vec3 vColor;
layout(location = 2) out vec3 vNormal;
layout(location = 3) out vec3 vViewDir;
layout(location = 4) out vec3 vWorldPosition;
layout(location = 5) out vec3 vCameraPosition;
layout(location = 6) out vec4 lightVertexColor;
layout(location = 7) out float visibility;

layout(binding = 0, std140) uniform SceneUniforms { 
	mat4 viewMatrix;
	mat4 projectionMatrix;
	vec3 eyePosition;
	float fogStart;
	float fogEnd;
	vec3 fogColor;
};

layout(binding = 1, std140) uniform ModelObjUniforms { 
	mat4 modelMatrix;
	mat3 normalMatrix;
};

layout(binding = 2, std140) uniform LightUniforms { 
	mat4 pointLights[8];
	int activeLights;
	float positionResolution;
};

void main()
{
	vTexCoords = aTexCoords;
	vColor     = aColor;
	//vNormal  = mat3(transpose(inverse(model))) * aNormal;
	//vNormal    = (modelMatrix * vec4(aNormal, 0)).xyz;
	vNormal = normalize(normalMatrix * aNormal);

	vec4 worldPosition = modelMatrix * vec4(aPosition, 1.0f);
	vWorldPosition = worldPosition.xyz;

	vec4 cameraPosition = viewMatrix * worldPosition;
	vCameraPosition = cameraPosition.xyz;

	vViewDir = normalize(eyePosition - vWorldPosition);

	gl_Position = projectionMatrix * cameraPosition;


	//Calculate visibility for vertex based on distance from camera
	float distanceFromCam = length(gl_Position.xyz);
	visibility = (distanceFromCam - fogStart) / (fogEnd - fogStart);
	visibility = clamp(visibility, 0.0, 1.0);
	visibility = 1.0 - visibility;
	visibility *= visibility;

	//recalculate distance for vertex for vertex jitter 
	distanceFromCam = clamp(gl_Position.w, -0.1, 1000.0);

	//apply nostalgic vertex jitter
	gl_Position.xy = round(gl_Position.xy * (positionResolution / distanceFromCam)) / (positionResolution / distanceFromCam);

	//Apply vertex lighting
	lightVertexColor = vec4(0);
	vec3 vertToLight;
	float dotToLight;
	float distToLight;
	float power;
	for (int i = 0; i < activeLights; i++)
	{
		vertToLight = pointLights[i][0].xyz - vWorldPosition;
		distToLight = length(vertToLight);
		power = (distToLight - pointLights[i][2][0]) / (pointLights[i][2][1] - pointLights[i][2][0]);//calc power based on distance to light and lights start/end distances.
		power = clamp(power, 0.0, 1.0);
		power = 1.0 - power;
		power *= power;
		dotToLight = clamp(dot(normalize(vertToLight), vNormal), 0.0, 1.0);
		lightVertexColor += vec4(aColor, 1.0) * pointLights[i][1] * dotToLight * pointLights[i][2][2] * power;
	}
	lightVertexColor.a = 1.0; // TODO: 
}