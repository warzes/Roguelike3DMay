#version 460 core

layout (location = 0) in vec3 aPosition;

layout(binding = 0, std140) uniform GlobalUniforms { 
	uniform mat4 view;
	uniform mat4 projection;
};

layout(binding = 1, std140) uniform ObjectUniforms { 
	uniform mat4 model;
};

void main()
{
	gl_Position = projection * view * model * vec4(aPosition, 1.0);
}