#include "stdafx.h"
#include "NewTest002.h"
//=============================================================================
// пример MSAA сглаживания
// - на основе предыдущего урока с выводом треугольника
// - добавлен UniformBuffer для передачи угла вращения
// - рендер в текстуру с мультисемплингом
// - блитинг текстуры на экран
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

	std::optional<gl::Buffer> vertexPosBuffer;
	std::optional<gl::Buffer> vertexColorBuffer;
	std::optional<gl::TypedBuffer<float>> timeBuffer;
	std::optional<gl::GraphicsPipeline> pipeline;
	std::optional<gl::Texture> msColorTex;
	std::optional<gl::Texture> resolveColorTex;

	double timeAccum = 0.0;
	gl::SampleCount numSamples = gl::SampleCount::Samples8;

	gl::GraphicsPipeline CreatePipeline()
	{
		auto descPos = gl::VertexInputBindingDescription{
		  .location = 0,
		  .binding = 0,
		  .format = gl::Format::R32G32_FLOAT,
		  .offset = 0,
		};
		auto descColor = gl::VertexInputBindingDescription{
		  .location = 1,
		  .binding = 1,
		  .format = gl::Format::R8G8B8_UNORM,
		  .offset = 0,
		};
		auto inputDescs = { descPos, descColor };

		auto vertexShader = gl::Shader(gl::PipelineStage::VertexShader, shaderCodeVertex, "Triangle VS");
		auto fragmentShader = gl::Shader(gl::PipelineStage::FragmentShader, shaderCodeFragment, "Triangle FS");

		return gl::GraphicsPipeline({
			 .name = "Triangle Pipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl::PrimitiveTopology::TriangleList},
			.vertexInputState = {inputDescs},
			});
	}

	void resize(uint16_t width, uint16_t height)
	{
		msColorTex = gl::Texture({
				.imageType = gl::ImageType::Tex2DMultisample,
				.format = gl::Format::R8G8B8A8_SRGB,
				.extent = {width / 8u, height / 8u},
				.mipLevels = 1,
				.arrayLayers = 1,
				.sampleCount = numSamples,
			});

		resolveColorTex = gl::Texture({
		  .imageType = gl::ImageType::Tex2D,
		  .format = gl::Format::R8G8B8A8_SRGB,
		  .extent = msColorTex->Extent(),
		  .mipLevels = 1,
		  .arrayLayers = 1,
		  .sampleCount = gl::SampleCount::Samples1,
			});
	}
}
//=============================================================================
EngineCreateInfo NewTest002::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool NewTest002::OnInit()
{
	static constexpr std::array<float, 6> triPositions = { -0.5, -0.5, 0.5, -0.5, 0.0, 0.5 };
	static constexpr std::array<uint8_t, 9> triColors = { 255, 0, 0, 0, 255, 0, 0, 0, 255 };
	vertexPosBuffer = gl::Buffer(triPositions);
	vertexColorBuffer = gl::Buffer(triColors);
	timeBuffer = gl::TypedBuffer<float>(gl::BufferStorageFlag::DynamicStorage);
	pipeline = CreatePipeline();

	resize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void NewTest002::OnClose()
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
	auto attachment = gl::RenderColorAttachment{
		.texture = msColorTex.value(),
		.loadOp = gl::AttachmentLoadOp::Clear,
		.clearValue = {.1f, .5f, .8f, 1.0f},
	};

	gl::BeginRendering({ .colorAttachments = {&attachment, 1}, });
	{
		gl::Cmd::BindGraphicsPipeline(pipeline.value());
		gl::Cmd::BindVertexBuffer(0, vertexPosBuffer.value(), 0, 2 * sizeof(float));
		gl::Cmd::BindVertexBuffer(1, vertexColorBuffer.value(), 0, 3 * sizeof(uint8_t));
		gl::Cmd::BindUniformBuffer(0, timeBuffer.value());
		gl::Cmd::Draw(3, 1, 0, 0);
	}
	gl::EndRendering();

	// Resolve multisample texture by blitting it to a same-size non-multisample texture
	gl::BlitTexture(*msColorTex, *resolveColorTex, {}, {}, msColorTex->Extent(), resolveColorTex->Extent(), gl::MagFilter::Linear);

	// Blit resolved texture to screen with nearest neighbor filter to make MSAA resolve more obvious
	gl::BlitTextureToSwapchain(*resolveColorTex,
		{},
		{},
		resolveColorTex->Extent(),
		{ GetWindowWidth(), GetWindowHeight(), 1},
		gl::MagFilter::Nearest);
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
	ImGui::Text("Max samples: %d", gl::gContext.properties.limits.maxSamples);
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
void NewTest002::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void NewTest002::OnMousePos(double x, double y)
{
}
//=============================================================================
void NewTest002::OnScroll(double dx, double dy)
{
}
//=============================================================================
void NewTest002::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================