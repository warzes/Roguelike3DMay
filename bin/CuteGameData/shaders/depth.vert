#version 460 core

layout(location = 0) in vec3 vertexPosition;

layout(binding = 0, std140) uniform DepthBlock {
	mat4 vp;
};

layout(binding = 1, std140) uniform ModelMatricesBlock {
	mat4 modelMatrix;
};

void main()
{
	gl_Position = vp * modelMatrix * vec4(vertexPosition, 1.0);
}