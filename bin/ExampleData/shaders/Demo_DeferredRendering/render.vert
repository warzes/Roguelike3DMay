#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec2 vertexTexCoord;
layout(location = 4) in vec3 vertexTangent;

layout(binding = 0, std140) uniform SceneDataBlock {
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 modelMatrix;
	vec3 cameraPosition;
};

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec4 fragWorldPosition;

out VS_OUT
{
	vec3 worldPosition;
	vec3 normal;
	vec3 tangent;
	vec2 texcoord0;
} vs_out;

void main()
{
	mat4 mv_matrix = viewMatrix * modelMatrix;
	vec4 worldPosition = modelMatrix * vec4(vertexPosition, 1.0);

	gl_Position = projectionMatrix * mv_matrix * vec4(vertexPosition, 1.0);
	vs_out.worldPosition = worldPosition.xyz;
	vs_out.normal = mat3(transpose(inverse(modelMatrix))) * vertexNormal;

	vs_out.tangent = vertexTangent;
	vs_out.texcoord0 = vertexTexCoord;
}