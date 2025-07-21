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

layout(binding = 0, std140) uniform GlobalUniforms { 
	uniform mat4 view;
	uniform mat4 projection;
	uniform vec3 eyePosition;
};

layout(binding = 1, std140) uniform ObjectUniforms { 
	uniform mat4 model;
	uniform int  numLight;
};

void main()
{
	vTexCoords = aTexCoords;
	vColor     = aColor;
	//vNormal  = mat3(transpose(inverse(model))) * aNormal;
	vNormal    = (model * vec4(aNormal, 0)).xyz;

	vec4 worldPosition = model * vec4(aPosition, 1.0f);
	vWorldPosition = worldPosition.xyz;

	vec4 cameraPosition = view * worldPosition;
	vCameraPosition = cameraPosition.xyz;

	vViewDir = normalize(eyePosition - vWorldPosition);

	gl_Position = projection * cameraPosition;
}