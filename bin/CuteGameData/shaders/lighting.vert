#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec2 vertexTexCoord;
layout(location = 4) in vec3 vertexTangent;

layout(binding = 0, std140) uniform SceneBlock {
	mat4 viewMatrix;
	mat4 projectionMatrix;
	vec3 cameraPosition;
};

layout(binding = 1, std140) uniform ModelMatricesBlock {
	mat4 modelMatrix;
};

layout(location = 0) out vec2 fragTexCoord; 
layout(location = 1) out vec3 fragNormal;        // Normal vector in world space
layout(location = 2) out vec4 fragWorldPosition; // Fragment position in world space
layout(location = 3) out vec3 fragViewDir;       // Direction vector from fragment to camera

void main()
{
	fragTexCoord       = vertexTexCoord;
	// Transform normal to world space and normalize
	fragNormal         = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
	// Calculate position in world space
	fragWorldPosition  = modelMatrix * vec4(vertexPosition, 1.0);
	// Calculate view direction vector
	fragViewDir        = normalize(cameraPosition - fragWorldPosition.xyz);

	// Transform vertex position into clip space
	gl_Position = projectionMatrix * viewMatrix * fragWorldPosition;
}