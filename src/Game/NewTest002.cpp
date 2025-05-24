#include "stdafx.h"
#include "NewTest002.h"
//=============================================================================
// This example renders a spinning triangle with MSAA.
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec3 a_color;

layout(location = 0) out vec3 v_color;

layout(binding = 0) uniform Uniforms { float time; };

void main()
{
	v_color = a_color;

	mat2 rot = mat2(
		cos(time), sin(time),
		-sin(time), cos(time)
	);

	gl_Position = vec4(rot * a_pos, 0.0, 1.0);
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

	std::optional<gl4::Buffer> vertexPosBuffer;
	std::optional<gl4::Buffer> vertexColorBuffer;
	std::optional<gl4::TypedBuffer<float>> timeBuffer;
	std::optional<gl4::GraphicsPipeline> pipeline;
	std::optional<gl4::Texture> msColorTex;
	std::optional<gl4::Texture> resolveColorTex;

	double timeAccum = 0.0;
	gl4::SampleCount numSamples = gl4::SampleCount::Samples8;

	gl4::GraphicsPipeline CreatePipeline()
	{
		auto descPos = gl4::VertexInputBindingDescription{
		  .location = 0,
		  .binding = 0,
		  .format = gl4::Format::R32G32_FLOAT,
		  .offset = 0,
		};
		auto descColor = gl4::VertexInputBindingDescription{
		  .location = 1,
		  .binding = 1,
		  .format = gl4::Format::R8G8B8_UNORM,
		  .offset = 0,
		};
		auto inputDescs = { descPos, descColor };

		auto vertexShader = gl4::Shader(gl4::PipelineStage::VertexShader, shaderCodeVertex, "Triangle VS");
		auto fragmentShader = gl4::Shader(gl4::PipelineStage::FragmentShader, shaderCodeFragment, "Triangle FS");

		return gl4::GraphicsPipeline({
			 .name = "Triangle Pipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl4::PrimitiveTopology::TRIANGLE_LIST},
			.vertexInputState = {inputDescs},
			});
	}

	void resize(uint16_t width, uint16_t height)
	{
		msColorTex = gl4::Texture({
				.imageType = gl4::ImageType::Tex2DMultisample,
				.format = gl4::Format::R8G8B8A8_SRGB,
				.extent = {width / 8u, height / 8u},
				.mipLevels = 1,
				.arrayLayers = 1,
				.sampleCount = numSamples,
			});

		resolveColorTex = gl4::Texture({
		  .imageType = gl4::ImageType::Tex2D,
		  .format = gl4::Format::R8G8B8A8_SRGB,
		  .extent = msColorTex->Extent(),
		  .mipLevels = 1,
		  .arrayLayers = 1,
		  .sampleCount = gl4::SampleCount::Samples1,
			});
	}
}
//=============================================================================
EngineConfig NewTest002::GetConfig() const
{
	return {};
}
//=============================================================================
bool NewTest002::OnCreate()
{
	static constexpr std::array<float, 6> triPositions = { -0.5, -0.5, 0.5, -0.5, 0.0, 0.5 };
	static constexpr std::array<uint8_t, 9> triColors = { 255, 0, 0, 0, 255, 0, 0, 0, 255 };
	vertexPosBuffer = gl4::Buffer(triPositions);
	vertexColorBuffer = gl4::Buffer(triColors);
	timeBuffer = gl4::TypedBuffer<float>(gl4::BufferStorageFlag::DynamicStorage);
	pipeline = CreatePipeline();

	resize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void NewTest002::OnDestroy()
{
	vertexPosBuffer = {};
	vertexColorBuffer = {};
	timeBuffer = {};
	pipeline = {};
	msColorTex = {};
	resolveColorTex = {};
}
//=============================================================================
void NewTest002::OnUpdate(float deltaTime)
{
	timeAccum += deltaTime * 0.02;
	timeBuffer->UpdateData(timeAccum);
}
//=============================================================================
void NewTest002::OnRender()
{
	auto attachment = gl4::RenderColorAttachment{
		.texture = msColorTex.value(),
		.loadOp = gl4::AttachmentLoadOp::Clear,
		.clearValue = {.1f, .5f, .8f, 1.0f},
	};

	gl4::BeginRendering({ .colorAttachments = {&attachment, 1}, });
	{
		gl4::Cmd::BindGraphicsPipeline(pipeline.value());
		gl4::Cmd::BindVertexBuffer(0, vertexPosBuffer.value(), 0, 2 * sizeof(float));
		gl4::Cmd::BindVertexBuffer(1, vertexColorBuffer.value(), 0, 3 * sizeof(uint8_t));
		gl4::Cmd::BindUniformBuffer(0, timeBuffer.value());
		gl4::Cmd::Draw(3, 1, 0, 0);
	}
	gl4::EndRendering();

	// Resolve multisample texture by blitting it to a same-size non-multisample texture
	gl4::BlitTexture(*msColorTex, *resolveColorTex, {}, {}, msColorTex->Extent(), resolveColorTex->Extent(), gl4::MagFilter::Linear);

	// Blit resolved texture to screen with nearest neighbor filter to make MSAA resolve more obvious
	gl4::BlitTextureToSwapchain(*resolveColorTex,
		{},
		{},
		resolveColorTex->Extent(),
		{ GetWindowWidth(), GetWindowHeight(), 1},
		gl4::MagFilter::Nearest);
}
//=============================================================================
void NewTest002::OnImGuiDraw()
{
	ImGui::Begin("Options");
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char*)glGetString(GL_VENDOR));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char*)glGetString(GL_VERSION));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char*)glGetString(GL_RENDERER));
	ImGui::Separator();
	ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Text("Framerate: %.0f Hertz", 1 / GetDeltaTime());
	ImGui::Text("Max samples: %d", gl4::gContext.properties.limits.maxSamples);
	ImGui::RadioButton("1 Sample", (int*)&numSamples, 1);
	ImGui::RadioButton("2 Samples", (int*)&numSamples, 2);
	ImGui::RadioButton("4 Samples", (int*)&numSamples, 4);
	ImGui::RadioButton("8 Samples", (int*)&numSamples, 8);
	ImGui::RadioButton("16 Samples", (int*)&numSamples, 16);
	ImGui::RadioButton("32 Samples", (int*)&numSamples, 32);
	ImGui::End();

	if (numSamples != msColorTex->GetCreateInfo().sampleCount)
	{
		resize(GetWindowWidth(), GetWindowHeight());
	}
}
//=============================================================================
void NewTest002::OnResize(uint16_t width, uint16_t height)
{
	resize(GetWindowWidth(), GetWindowHeight());
}
//=============================================================================