#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoords;
layout(location = 2) out vec4 worldPosition;
layout(location = 3) out float MaxNumLight;

layout(binding = 0, std140) uniform GlobalUniforms { 
	uniform mat4 view;
	uniform mat4 proj;
};

layout(binding = 1, std140) uniform ObjectUniforms { 
	uniform mat4 model;
	uniform float NumLight;
};

void main()
{
	gl_Position   = proj * view * model * vec4(aPosition, 1.0);

	fragNormal    = aNormal;//normalize(transpose(inverse(mat3(model))) * aNormal);
	fragTexCoords = aTexCoords;
	worldPosition = model * vec4(aPosition, 1.0f);
	MaxNumLight   = NumLight;
}