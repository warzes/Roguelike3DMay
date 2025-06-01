#include "stdafx.h"
#include "GameApp.h"

//=============================================================================
// ЗА ОСНОВУ TESTSHADOWMAPPING
// Nikola - шейдер света
// RendererGL - мелочь
// pbr
//плоскость как пол, возможно сверху куб
//камера и движение (на основе имеющегося)
//плоскость с текстурой
//что-нибудь из демо pbr (или потом)
//
//#version 460 core
//layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aNormal;
//layout (location = 2) in vec2 aTexCoord;
//
//layout (location = 0) out vec3 normal;
//layout (location = 1) out vec2 texCoord;
//
//layout (std140, binding = 0) uniform Matrices {
//    mat4 view;
//    mat4 projection;
//};
//
//void main() {
//  gl_Position = projection * view * vec4(aPos, 1.0);
//  normal = aNormal;
//  texCoord = aTexCoord;
//}
//
//#version 460 core
//layout (location = 0) in vec3 normal;
//layout (location = 1) in vec2 texCoord;
//
//out vec4 FragColor;
//
//uniform sampler2D tex;
//vec3 lightPos = vec3(4.0, 5.0, -3.0);
//vec3 lightColor = vec3(1.0, 1.0, 1.0);
//
//void main() {
//  float lightAngle = max(dot(normalize(normal), normalize(lightPos)), 0.0);
//  FragColor = texture(tex, texCoord) * vec4((0.3 + 0.7 * lightAngle) * lightColor, 1.0);
//}



//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;

layout(location = 0) out vec3 v_color;

layout(binding = 0) uniform Uniforms { 
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 proj;
};

void main()
{
	gl_Position = proj * view * model * vec4(a_pos, 1.0);
	v_color = a_color;
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

layout(location = 0) out vec4 o_color;

layout(location = 0) in vec3 v_color;

void main()
{
  o_color = vec4(v_color, 1.0);
}
)";

	struct Vertex final
	{
		glm::vec3 pos;
		glm::vec3 color;
	};

	struct UBO final
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	constexpr std::array<gl4::VertexInputBindingDescription, 2> inputBindingDescs{
	  gl4::VertexInputBindingDescription{
		.location = 0,
		.binding = 0,
		.format = gl4::Format::R32G32B32_FLOAT,
		.offset = offsetof(Vertex, pos),
	  },
	  gl4::VertexInputBindingDescription{
		.location = 1,
		.binding = 0,
		.format = gl4::Format::R32G32B32_FLOAT,
		.offset = offsetof(Vertex, color),
	  },
	};

	std::optional<gl4::Buffer> vertexBuffer1;
	std::optional<gl4::Buffer> indexBuffer;

	std::optional<gl4::TypedBuffer<UBO>> uniformBuffer1;
	std::optional<gl4::GraphicsPipeline> pipeline;
	std::optional<gl4::Texture> msColorTex;
	std::optional<gl4::Texture> gDepth;

	gl4::GraphicsPipeline CreatePipeline()
	{
		auto descPos = gl4::VertexInputBindingDescription{
		  .location = 0,
		  .binding = 0,
		  .format = gl4::Format::R32G32_FLOAT,
		  .offset = offsetof(Vertex, pos),
		};
		auto descColor = gl4::VertexInputBindingDescription{
		  .location = 1,
		  .binding = 0,
		  .format = gl4::Format::R32G32B32_FLOAT,
		  .offset = offsetof(Vertex, color),
		};
		auto inputDescs = { descPos, descColor };

		auto vertexShader = gl4::Shader(gl4::PipelineStage::VertexShader, shaderCodeVertex, "Triangle VS");
		auto fragmentShader = gl4::Shader(gl4::PipelineStage::FragmentShader, shaderCodeFragment, "Triangle FS");

		return gl4::GraphicsPipeline({
			 .name = "Triangle Pipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl4::PrimitiveTopology::TRIANGLE_LIST},
			.vertexInputState = {inputBindingDescs},
			.depthState = {.depthTestEnable = true},
			});
	}

	void resize(uint16_t width, uint16_t height)
	{
		// размер уменьшить на 8
		msColorTex = gl4::CreateTexture2D({ width, height }, gl4::Format::R8G8B8A8_SRGB, "gAlbedo");

		gDepth = gl4::CreateTexture2D({ width, height }, gl4::Format::D32_FLOAT, "gDepth");
	}
}
//=============================================================================
EngineCreateInfo GameApp::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool GameApp::OnInit()
{
	std::vector<Vertex> v = {
		{{  0.0f,  0.4f, 1.0f}, {1, 0, 0}},
		{{ -1.0f, -1.0f, 1.0f}, {0, 1, 0}},
		{{  1.0f, -1.0f, 1.0f}, {0, 0, 1}},
	};
	vertexBuffer1 = gl4::Buffer(std::span(v));

	std::vector<uint32_t> iv = { 0, 1, 2 };
	indexBuffer = gl4::Buffer(std::span(iv));

	uniformBuffer1 = gl4::TypedBuffer<UBO>(gl4::BufferStorageFlag::DynamicStorage);

	pipeline = CreatePipeline();


	resize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void GameApp::OnClose()
{
	vertexBuffer1 = {};
	indexBuffer = {};
	uniformBuffer1 = {};
	uniformBuffer1 = {};
	pipeline = {};
	msColorTex = {};
}
//=============================================================================
void GameApp::OnUpdate(float deltaTime)
{
	UBO ubo;
	ubo.model = glm::mat4(1.0f);
	ubo.view = glm::mat4(1.0f);
	ubo.proj = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.01f, 1000.0f);


	uniformBuffer1->UpdateData(ubo);
}
//=============================================================================
void GameApp::OnRender()
{
	auto attachment = gl4::RenderColorAttachment{
		.texture = msColorTex.value(),
		.loadOp = gl4::AttachmentLoadOp::Clear,
		.clearValue = {.1f, .5f, .8f, 1.0f},
	};

	auto gDepthAttachment = gl4::RenderDepthStencilAttachment{
	  .texture = gDepth.value(),
	  .loadOp = gl4::AttachmentLoadOp::Clear,
	  .clearValue = {.depth = 1.0f},
	};

	gl4::BeginRendering({
		.colorAttachments = {&attachment, 1},
		.depthAttachment = gDepthAttachment
		});
	{
		gl4::Cmd::BindGraphicsPipeline(pipeline.value());
		gl4::Cmd::BindVertexBuffer(0, vertexBuffer1.value(), 0, sizeof(Vertex));
		gl4::Cmd::BindIndexBuffer(indexBuffer.value(), gl4::IndexType::UNSIGNED_INT);
		gl4::Cmd::BindUniformBuffer(0, uniformBuffer1.value());
		gl4::Cmd::DrawIndexed(3, 1, 0, 0, 0);
	}
	gl4::EndRendering();

	gl4::BlitTextureToSwapchain(*msColorTex,
		{},
		{},
		msColorTex->Extent(),
		{ GetWindowWidth(), GetWindowHeight(), 1 },
		gl4::MagFilter::Nearest);
}
//=============================================================================
void GameApp::OnImGuiDraw()
{
	//ImGui::Begin("Simple");
	//ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char*)glGetString(GL_VENDOR));
	//ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char*)glGetString(GL_VERSION));
	//ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char*)glGetString(GL_RENDERER));
	//ImGui::Separator();
	////ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//ImGui::End();

	//DrawProfilerInfo();
	DrawFPS();
}
//=============================================================================
void GameApp::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================
void GameApp::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void GameApp::OnMousePos(double x, double y)
{
}
//=============================================================================
void GameApp::OnScroll(double dx, double dy)
{
}
//=============================================================================
void GameApp::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================