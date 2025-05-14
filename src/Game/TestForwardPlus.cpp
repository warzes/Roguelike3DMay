#include "stdafx.h"
#include "TestForwardPlus.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexCoords;

out VS_DATA
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vsOut;

layout(std140, binding = 0) uniform MVPData {
	mat4 model;
	mat4 view;
	mat4 projection;
};

void main()
{
	vsOut.FragPos = mat3(model) * aVertexPosition;
	vsOut.Normal = transpose(inverse(mat3(model))) * aVertexNormal;
	vsOut.TexCoords = aVertexTexCoords;
	gl_Position = projection * view * model * vec4(aVertexPosition, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

in VS_DATA
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fsIn;

layout(std140, binding = 1) uniform LightData {
	vec3 lightPos;
	vec3 viewPos;
};

layout(binding = 0) uniform sampler2D diffuseTexture1;

out vec4 FragColorOut;

// Blinn-Phong with glTF model
void main()
{
	vec3 color = texture(diffuseTexture1, fsIn.TexCoords).rgb;

	// ambient
	vec3 ambient = 0.05 * color;

	// diffuse
	vec3 lightDir = normalize(lightPos - fsIn.FragPos);
	vec3 normal = normalize(fsIn.Normal);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * color;

	// specular
	vec3 viewDir = normalize(viewPos - fsIn.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;

	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	vec3 specular = vec3(0.3) * spec; // assuming bright white light color
	FragColorOut = vec4(ambient + diffuse + specular, 1.0);
}
)";

	const char* lightShaderCodeVertex = R"(
#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";
	const char* lightShaderCodeFragment = R"(
#version 460 core
out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 0.8, 0.2, 1.0);
}
)";
	
	struct alignas(16) MVPData final
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
	};

	struct alignas(16) Light2Data final
	{
		glm::vec3 lightPos;
		glm::vec3 viewPos;
	};

	GLuint program;

	MVPData mvpData;
	GLuint mvpUbo;

	Light2Data lightData;
	GLuint lightUbo;

	GLuint lightProgram;
	int lightModelLoc;
	int lightViewLoc;
	int lightProjLoc;

	GLuint texture;
	GLuint vbo;
	GLuint vao;

	Camera camera;

	glm::vec3 lightPos(1.0f, 0.5f, 0.0f);

	Model* model;

#pragma region Forward+
	const char* depthShaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 aVertexPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aVertexPosition, 1.0);
}
)";

	const char* depthShaderCodeFragment = R"(
#version 460 core

void main()
{
}
)";

	const char* depthDebugShaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 aVertexPosition;
layout (location = 1) in vec3 aVertexNormal;
layout (location = 2) in vec2 aVertexTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;

void main()
{
	gl_Position = projection * view * model * vec4(aVertexPosition, 1.0);
	TexCoords = aVertexTexCoords;
}
)";

	const char* depthDebugShaderCodeFragment = R"(
#version 460 core

uniform float near;
uniform float far;

out vec4 fragColor;

// Need to linearize the depth because we are using the projection
float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
	float depth = LinearizeDepth(gl_FragCoord.z) / far;
	fragColor = vec4(vec3(depth), 1.0);
}
)";

	const char* lightCullingShaderCodeCompute = R"(
#version 460 core

#define MAX_LIGHT_PER_TILE 128
#define TILE_SIZE 16

struct PointLight
{
	vec4 position;
	vec4 color;
	vec4 paddingAndRadius;
};

// SSBO
layout(std430, binding = 0) readonly buffer LightBuffer {
	PointLight data[];
};

layout(std430, binding = 1) writeonly buffer visible_lights_indices {
	int lights_indices[];
};

// Uniforms
uniform sampler2D depthMap;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 screenSize;
uniform int lightCount;

// Shared values between all threads in the group
shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visibleLightCount;
shared vec4 frustumPlanes[6];
// Shared local storage for visible indices, will be written out to the global buffer at the end
shared int visibleLightIndices[MAX_LIGHT_PER_TILE];
// shared mat viewProjection;

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE) in;

void main()
{
	ivec2 tile_id = ivec2(gl_WorkGroupID.xy);

	if (gl_LocalInvocationIndex == 0)
	{
		minDepthInt = 0xFFFFFFFF;
		maxDepthInt = 0x0;
		visibleLightCount = 0;
		uint index = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
		uint offset = index * MAX_LIGHT_PER_TILE;
		for(uint i = 0; i < MAX_LIGHT_PER_TILE; i++) {
			lights_indices[offset + i] = -1;
		}
	}
	barrier();

	// Compute depth min and max of the workgroup
	vec2 screen_uv = gl_GlobalInvocationID.xy / screenSize;

	float depth = texture
}
)";

	const char* lightingShaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 aPos;

void main()
{
}
)";

	const char* lightingShaderCodeFragment = R"(
#version 460 core

void main()
{
}
)";

	const char* finalShaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 aPos;

void main()
{
}
)";

	const char* finalShaderCodeFragment = R"(
#version 460 core

void main()
{
}
)";

	struct alignas(16) LightData final
	{
		glm::vec4 position;
		glm::vec4 color;
		glm::vec4 paddingAndRadius;
	};

	GLuint depthProgram;
	GLuint depthDebugProgram;
	GLuint lightCullingProgram;
	GLuint lightingProgram;
	GLuint finalProgram;

	int numberOfLight{ 35 };

	GLuint lightBufferSSBO;
	GLuint lightIndexBufferSSBO;

#define MAX_LIGHT_PER_TILE 128
#define TILE_SIZE 16

	const glm::vec3 minLightBoundaries{ -5.0f, -5.0f, -25.0f };
	const glm::vec3 maxLightBoundaries{ 5.0f, 10.0f, 25.0f };

	GLuint workGroupsX;
	GLuint workGroupsY;


#pragma endregion



}
//=============================================================================
EngineConfig TestForwardPlus::GetConfig() const
{
	return {};
}
//=============================================================================
bool TestForwardPlus::OnCreate()
{
	program = gl4::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);

	mvpUbo = gl4::CreateBuffer(GL_DYNAMIC_STORAGE_BIT, sizeof(MVPData), nullptr);
	lightUbo = gl4::CreateBuffer(GL_DYNAMIC_STORAGE_BIT, sizeof(Light2Data), nullptr);

	lightProgram = gl4::CreateShaderProgram(lightShaderCodeVertex, lightShaderCodeFragment);
	lightModelLoc = gl4::GetUniformLocation(lightProgram, "model");
	lightViewLoc = gl4::GetUniformLocation(lightProgram, "view");
	lightProjLoc = gl4::GetUniformLocation(lightProgram, "projection");

	texture = gl4::LoadTexture2D("data/textures/wood.png", false);

	model = new Model("data/mesh/Sponza/Sponza.gltf");

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	std::vector<gl4::VertexAttribute> attribs = {
		{0, 3, GL_FLOAT, false, offsetof(Vertex, pos)},
		{1, 3, GL_FLOAT, false, offsetof(Vertex, normal)},
		{2, 2, GL_FLOAT, false, offsetof(Vertex, uv)},
	};

	// Quad
	float vertices[]{
		// positions            // normals            // texcoords
		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
		 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
	};

	vbo = gl4::CreateBuffer(0, sizeof(vertices), vertices);
	vao = gl4::CreateVertexArray(vbo, sizeof(Vertex), attribs);

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

#pragma region Forward+

	depthProgram = gl4::CreateShaderProgram(depthShaderCodeVertex, depthShaderCodeFragment);
	depthDebugProgram = gl4::CreateShaderProgram(depthDebugShaderCodeVertex, depthDebugShaderCodeFragment);
	lightCullingProgram = gl4::CreateShaderProgram(lightCullingShaderCodeCompute);
	lightingProgram = gl4::CreateShaderProgram(lightingShaderCodeVertex, lightingShaderCodeFragment);
	finalProgram = gl4::CreateShaderProgram(finalShaderCodeVertex, finalShaderCodeFragment);

	workGroupsX = (GetWidth() + (GetWidth() % TILE_SIZE)) / TILE_SIZE;
	workGroupsY = (GetHeight() + (GetHeight() % TILE_SIZE)) / TILE_SIZE;

	GLuint tilesCount = workGroupsX * workGroupsY;

	// create light buffer
	lightBufferSSBO = gl4::CreateBuffer(GL_MAP_WRITE_BIT, numberOfLight * sizeof(LightData), nullptr);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBufferSSBO);
	// create visible light indices buffer
	lightIndexBufferSSBO = gl4::CreateBuffer(GL_MAP_WRITE_BIT, sizeof(int) * tilesCount * MAX_LIGHT_PER_TILE, nullptr);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightIndexBufferSSBO);

	// Generate lights
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 1);

		LightData* lights = (LightData*)glMapNamedBufferRange(lightBufferSSBO, 0, numberOfLight * sizeof(LightData), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		for (size_t i = 0; i < numberOfLight; i++)
		{
			LightData& light = lights[i];

			light.position.x = minLightBoundaries.x + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxLightBoundaries.x - minLightBoundaries.x)));
			light.position.y = minLightBoundaries.y + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxLightBoundaries.y - minLightBoundaries.y)));
			light.position.z = minLightBoundaries.z + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxLightBoundaries.z - minLightBoundaries.z)));
			light.position.w = 1.0f;

			light.color = {
				static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
				static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
				static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
				1.0f
			};

			light.paddingAndRadius = { 0.0f, 0.0f, 0.0f, 8.0f };
		}

		glUnmapNamedBuffer(lightBufferSSBO);
	}
#pragma endregion

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	//glEnable(GL_CULL_FACE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}
//=============================================================================
void TestForwardPlus::OnDestroy()
{
	glDeleteTextures(1, &texture);
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}
//=============================================================================
void TestForwardPlus::OnUpdate(float deltaTime)
{
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraForward, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraBackward, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraLeft, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraRight, deltaTime);

	if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		SetCursorVisible(false);
		camera.ProcessMouseMovement(-GetMouseDeltaX(), -GetMouseDeltaY());
	}
	else if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		glfwSetInputMode(GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		SetCursorVisible(true);
	}
}
//=============================================================================
void TestForwardPlus::OnRender()
{
	gl4::SetFrameBuffer(0, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mvpData.model = glm::mat4(1.0f);
	mvpData.view = camera.GetViewMatrix();
	mvpData.projection = glm::perspective(glm::radians(60.0f), GetAspect(), 0.01f, 1000.0f);

	lightData.lightPos = lightPos;
	lightData.viewPos = camera.Position;

	glNamedBufferSubData(mvpUbo, 0, sizeof(MVPData), &mvpData);
	glNamedBufferSubData(lightUbo, 0, sizeof(Light2Data), &lightData);

	// вывод квада плоскости
	{
		glUseProgram(program);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, mvpUbo);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightUbo);
		glBindTextureUnit(0, texture);
		glBindVertexArray(vao);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	// вывод модели
	{
		glUseProgram(program);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, mvpUbo);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightUbo);
		model->Draw(program);
	}

	// рендер источника света
	{
		glUseProgram(lightProgram);
		gl4::SetUniform(lightProjLoc, mvpData.projection);
		gl4::SetUniform(lightViewLoc, mvpData.view);

		glm::mat4 modelMat = glm::mat4(1.0f);
		modelMat = glm::translate(modelMat, lightPos);
		modelMat = glm::scale(modelMat, glm::vec3(0.2f));
		gl4::SetUniform(lightModelLoc, modelMat);

		GetGraphicSystem().DrawCube();
	}
}
//=============================================================================
void TestForwardPlus::OnImGuiDraw()
{
}
//=============================================================================
void TestForwardPlus::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================