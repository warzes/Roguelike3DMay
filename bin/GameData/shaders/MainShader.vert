#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoords;
layout (location = 4) in vec3 aTangent;

layout(location = 0) out vec3 FragPosition;
layout(location = 1) out vec3 FragColor;
layout(location = 2) out vec3 FragNormal;
layout(location = 3) out vec2 FragTexCoords;
layout(location = 4) out vec3 vViewDir;
layout(binding = 0, std140) uniform GlobalUniforms { 
	uniform mat4 view;
	uniform mat4 projection;
	uniform vec3 eyePosition;
};

layout(binding = 1, std140) uniform ObjectUniforms { 
	uniform mat4 model;
};

void main()
{
	FragPosition  = vec3(model * vec4(aPosition, 1.0));
	FragColor     = aColor;
	FragNormal    = mat3(transpose(inverse(model))) * aNormal;
	FragTexCoords = aTexCoords;

	vViewDir      = normalize(eyePosition - FragPosition);

	gl_Position = projection * view * vec4(FragPosition, 1.0);
}