#version 460 core

layout (location = 0) in vec3 aPosition;

layout(binding = 0, std140) uniform Uniforms { 
	mat4 vp;
	mat4 model;
};

void main()
{
	gl_Position = vp * model * vec4(aPosition, 1.0);
}