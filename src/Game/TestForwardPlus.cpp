#include "stdafx.h"
#include "TestForwardPlus.h"

// TODO: выделить стадию рендера в глубину, так как она универсальна
//=============================================================================
namespace
{
	Camera camera;
	Model* model;

#define MAX_POINT_LIGHT_PER_TILE 2049
#define TILE_SIZE 16

#pragma region depthDebugShader
	const char* depthDebugShaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 position;

uniform mat4 VP;
uniform mat4 model;

void main() {
	gl_Position = VP * model * vec4(position, 1.0);
}
)";

	const char* depthDebugShaderCodeFragment = R"(
#version 460 core

uniform float near;
uniform float far;

out vec4 fragColor;

// Need to linearize the depth because we are using the projection
float LinearizeDepth(float depth) {
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
	float depth = LinearizeDepth(gl_FragCoord.z) / far;
	fragColor = vec4(vec3(depth), 1.0f);
}
)";
#pragma endregion

	struct alignas(16) PointLight final
	{
		glm::vec3 position;
		float radius;
		glm::vec3 color;
		float padding;
	};

	struct DummyVisibleLightsForTile final
	{
		uint32_t count;
		std::array<uint32_t, MAX_POINT_LIGHT_PER_TILE> lightIndices;
	};

	DepthPrepass depthPrepass;
	
	gl4::ShaderProgramId depthDebugProgram;
	
	gl4::ShaderProgramId lightCullingProgram;
	int lightCullingInvViewProjectionLoc;
	int lightCullingCamPosLoc;
	int lightCullingLightCountLoc;
	int lightCullingViewportSizeLoc;
	int lightCullingTileNumsLoc;


	gl4::ShaderProgramId lightProgram;
	int lightProgramProjectionLoc;
	int lightProgramViewLoc;
	int lightProgramModelLoc;
	int lightProgramProjViewLoc;
	int lightProgramViewPositionLoc;
	int lightProgramViewportSizeLoc;
	int lightProgramTileNumsLoc;
	int lightProgramLightDebugLoc;


	GLuint SSBOLights;
	GLuint SSBOVisibleLights;

	uint32_t numberOfLights{ 1000 };

	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	const glm::vec3 minLightBoundaries{ -15.0f, -5.0f, -25.0f };
	const glm::vec3 maxLightBoundaries{ 15.0f, 20.0f, 25.0f };

	GLuint tileCountPerRow;
	GLuint tileCountPerCol;

	gl4::FrameBufferId renderFBO;
	GLuint rboColorBuffer;
	GLuint rboDepthBuffer;


	enum Modes
	{
		SHADED,
		DEPTH,
		LIGHT
	} ViewModes;

	void ReCreateForwardPlusFBO(uint16_t width, uint16_t height)
	{
		if (rboColorBuffer) glDeleteRenderbuffers(1, &rboColorBuffer);
		if (rboDepthBuffer) glDeleteRenderbuffers(1, &rboDepthBuffer);
		if (renderFBO) gl4::Destroy(renderFBO);

		glCreateRenderbuffers(1, &rboColorBuffer);
		glNamedRenderbufferStorage(rboColorBuffer, GL_RGB32F, width, height);

		glCreateRenderbuffers(1, &rboDepthBuffer);
		glNamedRenderbufferStorage(rboDepthBuffer, GL_DEPTH_COMPONENT32, width, height);

		glDeleteFramebuffers(1, &renderFBO.id);
		glNamedFramebufferRenderbuffer(renderFBO, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboColorBuffer);
		glNamedFramebufferRenderbuffer(renderFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthBuffer);

		const GLenum status = glCheckNamedFramebufferStatus(renderFBO, GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			// TODO: error
		}
	}

	void SetupLights()
	{
		if (SSBOLights == 0) return;

		PointLight* lights = (PointLight*)glMapNamedBuffer(SSBOLights, GL_WRITE_ONLY);

		for (size_t i = 0; i < numberOfLights; i++)
		{
			PointLight& light = lights[i];

			light.position.x = minLightBoundaries.x + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxLightBoundaries.x - minLightBoundaries.x)));
			light.position.y = minLightBoundaries.y + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxLightBoundaries.y - minLightBoundaries.y)));
			light.position.z = minLightBoundaries.z + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxLightBoundaries.z - minLightBoundaries.z)));

			light.color = {
				static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
				static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
				static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
			};

			light.radius = 5.0f;
		}

		glUnmapNamedBuffer(SSBOLights);
	}

	void UpdateLights(float deltaTime)
	{
		PointLight* lights = (PointLight*)glMapNamedBuffer(SSBOLights, GL_WRITE_ONLY);

		for (int i = 0; i < numberOfLights; i++)
		{
			PointLight& light = lights[i];
			light.position += glm::vec3(0, 3.0f, 0) * deltaTime;
			if (light.position.y > maxLightBoundaries.y)
			{
				light.position.y -= (maxLightBoundaries.y- minLightBoundaries.y);
			}
		}

		glUnmapNamedBuffer(SSBOLights);
	}
}
//=============================================================================
EngineConfig TestForwardPlus::GetConfig() const
{
	return {};
}
//=============================================================================
bool TestForwardPlus::OnCreate()
{
	model = new Model("ExampleData/mesh/Sponza/Sponza.gltf");
	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	ViewModes = Modes::SHADED;

	tileCountPerRow = (GetWidth() - 1) / TILE_SIZE + 1;
	tileCountPerCol = (GetHeight() - 1) / TILE_SIZE + 1;

	GLuint tilesCount = tileCountPerRow * tileCountPerCol;

	depthPrepass.Create(GetWidth(), GetHeight());
	depthDebugProgram = gl4::CreateShaderProgram(depthDebugShaderCodeVertex, depthDebugShaderCodeFragment);
	lightCullingProgram = gl4::CreateShaderProgram(FileUtils::ReadShaderCode("ExampleData/shaders/TestForwardPlus/lightculling.comp", {}).c_str());

	lightCullingInvViewProjectionLoc = gl4::GetUniformLocation(lightCullingProgram, "invViewProjection");
	lightCullingCamPosLoc = gl4::GetUniformLocation(lightCullingProgram, "cameraPosition");
	lightCullingLightCountLoc = gl4::GetUniformLocation(lightCullingProgram, "lightNum");
	lightCullingViewportSizeLoc = gl4::GetUniformLocation(lightCullingProgram, "viewportSize");
	lightCullingTileNumsLoc = gl4::GetUniformLocation(lightCullingProgram, "tileNums");

	lightProgram = gl4::CreateShaderProgram(
		FileUtils::ReadShaderCode("ExampleData/shaders/TestForwardPlus/forwardplus.vert", {}).c_str(), 
		FileUtils::ReadShaderCode("ExampleData/shaders/TestForwardPlus/forwardplus.frag", {}).c_str());

	lightProgramProjectionLoc = gl4::GetUniformLocation(lightProgram, "projection");
	lightProgramViewLoc = gl4::GetUniformLocation(lightProgram, "view");
	lightProgramModelLoc = gl4::GetUniformLocation(lightProgram, "model");
	lightProgramProjViewLoc = gl4::GetUniformLocation(lightProgram, "projView");
	lightProgramViewPositionLoc = gl4::GetUniformLocation(lightProgram, "viewPosition");
	lightProgramViewportSizeLoc = gl4::GetUniformLocation(lightProgram, "viewportSize");
	lightProgramTileNumsLoc = gl4::GetUniformLocation(lightProgram, "tileNums");
	lightProgramLightDebugLoc = gl4::GetUniformLocation(lightProgram, "debugViewIndex");

	// create light buffer
	SSBOLights = gl4::CreateBufferStorage(GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, MAX_POINT_LIGHT_PER_TILE * sizeof(PointLight), nullptr);
	SSBOVisibleLights = gl4::CreateBuffer(GL_DYNAMIC_DRAW, tileCountPerRow * tileCountPerCol * sizeof(DummyVisibleLightsForTile), nullptr);

	// Generate lights
	SetupLights();
	
	// Output buffer + color
	ReCreateForwardPlusFBO(GetWidth(), GetHeight());

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);

	//glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}
//=============================================================================
void TestForwardPlus::OnDestroy()
{
	depthPrepass.Destroy();
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
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), GetAspect(), nearPlane, farPlane);
	glm::mat4 viewProjection = projection * view;
	glm::mat4 invViewProjection = glm::inverse(viewProjection);
	glm::mat4 modelMat = glm::mat4(1.0f);

	UpdateLights(GetDeltaTime());

	// 1) Depth prepass
	{
		//glEnable(GL_POLYGON_OFFSET_FILL);
		//glPolygonOffset(4.0f, 4.0f);

		depthPrepass.Start(GetWidth(), GetHeight(), viewProjection);
		depthPrepass.DrawModel(model, modelMat); // TODO: модельную матрицу добавить в модель
			
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glPolygonOffset(0.0f, 0.0f);
		//glDisable(GL_POLYGON_OFFSET_FILL);
	}

	// 1.1) Depth debug
	if (ViewModes == DEPTH)
	{
		glUseProgram(depthDebugProgram);
		gl4::SetUniform(depthDebugProgram, "VP", viewProjection);
		gl4::SetUniform(depthDebugProgram, "model", modelMat);
		gl4::SetUniform(depthDebugProgram, "near", nearPlane);
		gl4::SetUniform(depthDebugProgram, "far", farPlane);
		gl4::SetFrameBuffer({ 0 }, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		model->Draw(depthDebugProgram, true);
	}
	else
	{
		// 2) Light Culling Compute
		{
			glUseProgram(lightCullingProgram);

			gl4::SetUniform(lightCullingProgram, lightCullingInvViewProjectionLoc, invViewProjection);
			gl4::SetUniform(lightCullingProgram, lightCullingCamPosLoc, camera.Position);
			gl4::SetUniform(lightCullingProgram, lightCullingLightCountLoc, numberOfLights);
			gl4::SetUniform(lightCullingProgram, lightCullingViewportSizeLoc, glm::ivec2(GetWidth(), GetHeight()));
			gl4::SetUniform(lightCullingProgram, lightCullingTileNumsLoc, glm::ivec2(tileCountPerRow, tileCountPerCol));

			depthPrepass.BindTexture(0);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBOLights);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBOVisibleLights);

			glDispatchCompute(tileCountPerRow, tileCountPerCol, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}

		// 3) Light Pass
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			gl4::SetFrameBuffer(renderFBO, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(lightProgram);
			gl4::SetUniform(lightProgram, lightProgramProjectionLoc, projection);
			gl4::SetUniform(lightProgram, lightProgramViewLoc, view);
			gl4::SetUniform(lightProgram, lightProgramModelLoc, modelMat);
			gl4::SetUniform(lightProgram, lightProgramProjViewLoc, viewProjection);
			gl4::SetUniform(lightProgram, lightProgramViewPositionLoc, camera.Position);
			gl4::SetUniform(lightProgram, lightProgramViewportSizeLoc, glm::ivec2(GetWidth(), GetHeight()));
			gl4::SetUniform(lightProgram, lightProgramTileNumsLoc, glm::ivec2(tileCountPerRow, tileCountPerCol));
			gl4::SetUniform(lightProgram, lightProgramLightDebugLoc, (ViewModes == LIGHT) ? 1 : 0);

			model->Draw(lightProgram);
		}

		// 4) final draw
		{
			glBlitNamedFramebuffer(renderFBO, 0, 0, 0, GetWidth(), GetHeight(), 0, 0, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}
	}
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
}
//=============================================================================
void TestForwardPlus::OnImGuiDraw()
{
	// Create a simple ImGUI config window.
	{
		ImGui::Begin("Forward+ Shading");

		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char*)glGetString(GL_VENDOR));
		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char*)glGetString(GL_VERSION));
		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char*)glGetString(GL_RENDERER));
		ImGui::Separator();

		ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::Separator();

		ImGui::Text("Tile size: %ix%i", TILE_SIZE, TILE_SIZE);
		ImGui::Text("Max lights per tile: %i", MAX_POINT_LIGHT_PER_TILE);

		if (ImGui::CollapsingHeader("View Mode"))
		{

			const char* items[] = { "Shaded", "Depth", "Light Debug" };
			static int item_current = 0;
			ImGui::Combo("Mode", &item_current, items, IM_ARRAYSIZE(items));

			ViewModes = static_cast<Modes>(item_current);

		}

		int numLight = numberOfLights;
		if (ImGui::SliderInt("Lights count", &numLight, 1, MAX_POINT_LIGHT_PER_TILE -1))
		{
			SetupLights();
			glUseProgram(lightCullingProgram);
			glUniform1ui(glGetUniformLocation(lightCullingProgram, "lightCount"), numberOfLights);
			numberOfLights = numLight;
		}

		if (ImGui::Button("Recalculate lights"))
		{
			UpdateLights(GetDeltaTime());
			SetupLights();
			glUseProgram(lightCullingProgram);
			glUniform1ui(glGetUniformLocation(lightCullingProgram, "lightCount"), numberOfLights);
		}

		ImGui::End();
	}
}
//=============================================================================
void TestForwardPlus::OnResize(uint16_t width, uint16_t height)
{
	ReCreateForwardPlusFBO(width, height);
	tileCountPerRow = (width - 1) / TILE_SIZE + 1;
	tileCountPerCol = (height - 1) / TILE_SIZE + 1;

	glNamedBufferData(SSBOVisibleLights, tileCountPerRow * tileCountPerCol * sizeof(DummyVisibleLightsForTile), nullptr, GL_DYNAMIC_DRAW);
}
//=============================================================================