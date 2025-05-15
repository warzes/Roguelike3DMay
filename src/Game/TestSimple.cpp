#include "stdafx.h"
#include "TestSimple.h"
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

	GLuint lightDrawProgram;
	int lightModelLoc;
	int lightViewLoc;
	int lightProjLoc;

	GLuint texture;
	GLuint vbo;
	GLuint vao;

	Camera camera;

	glm::vec3 lightPos(1.0f, 0.5f, 0.0f);

	Model* model;
}
//=============================================================================
EngineConfig TestSimple::GetConfig() const
{
	return {};
}
//=============================================================================
bool TestSimple::OnCreate()
{
	program = gl4::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);

	mvpUbo = gl4::CreateBufferStorage(GL_DYNAMIC_STORAGE_BIT, sizeof(MVPData), nullptr);
	lightUbo = gl4::CreateBufferStorage(GL_DYNAMIC_STORAGE_BIT, sizeof(Light2Data), nullptr);

	lightDrawProgram = gl4::CreateShaderProgram(lightShaderCodeVertex, lightShaderCodeFragment);
	lightModelLoc = gl4::GetUniformLocation(lightDrawProgram, "model");
	lightViewLoc = gl4::GetUniformLocation(lightDrawProgram, "view");
	lightProjLoc = gl4::GetUniformLocation(lightDrawProgram, "projection");

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

	vbo = gl4::CreateBufferStorage(0, sizeof(vertices), vertices);
	vao = gl4::CreateVertexArray(vbo, sizeof(Vertex), attribs);

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}
//=============================================================================
void TestSimple::OnDestroy()
{
	glDeleteTextures(1, &texture);
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}
//=============================================================================
void TestSimple::OnUpdate(float deltaTime)
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
void TestSimple::OnRender()
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

	 //вывод модели
	{
		glUseProgram(program);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, mvpUbo);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightUbo);
		model->Draw(program);
	}

	// рендер источника света
	{
		glUseProgram(lightDrawProgram);
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
void TestSimple::OnImGuiDraw()
{
	ImGui::Begin("Simple");

	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char*)glGetString(GL_VENDOR));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char*)glGetString(GL_VERSION));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char*)glGetString(GL_RENDERER));
	ImGui::Separator();

	ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}
//=============================================================================
void TestSimple::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================