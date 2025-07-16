#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoords;
layout (location = 4) in vec3 aTangent;

layout(binding = 0, std140) uniform GlobalUniforms { 
	uniform mat4 view;
	uniform mat4 projection;
};

layout(binding = 2, std140) uniform ObjectUniforms { 
	uniform mat4 model;
};

void main()
{
	gl_Position = projection * view * model * vec4(aPosition, 1.0);
}