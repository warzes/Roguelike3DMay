#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// Interface block
out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

layout(binding = 0) uniform Matrices { 
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 proj;
};

void main()
{
	vs_out.FragPos = aPos;
	vs_out.Normal = aNormal; // TODO transpose(inverse(mat3(model))) * aNormal;
	vs_out.TexCoords = aTexCoords;
	gl_Position = proj * view * model * vec4(aPos, 1.0);
}