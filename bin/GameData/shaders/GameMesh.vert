#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

layout(location = 0) out vec3 FragPosition;
layout(location = 1) out vec3 FragNormal;
layout(location = 2) out vec2 FragTexCoords;
layout(location = 3) out vec3 TangentViewPos;
layout(location = 4) out vec3 TangentFragPos;

layout(binding = 0, std140) uniform GlobalUniforms { 
	uniform mat4 view;
	uniform mat4 projection;
};

layout(binding = 1, std140) uniform SceneUniforms { 
	uniform int NumDirectionalLight;
	uniform int NumPointLight;
	uniform vec3 cameraPos;
};

layout(binding = 2, std140) uniform ObjectUniforms { 
	uniform mat4 model;
};

void main()
{
	FragPosition = vec3(model * vec4(aPosition, 1.0));
	FragNormal   = mat3(transpose(inverse(model))) * aNormal;
	FragTexCoords = aTexCoords;

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 T = normalize(normalMatrix * aTangent);
	vec3 N = normalize(normalMatrix * aNormal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	mat3 TBN = transpose(mat3(T, B, N));
	TangentViewPos  = TBN * cameraPos;
	TangentFragPos  = TBN * FragPosition;

	gl_Position = projection * view * vec4(FragPosition, 1.0);
}