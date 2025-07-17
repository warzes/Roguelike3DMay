#include "stdafx.h"
#include "NewTest004.h"
#include "RsmTechnique.h"
/*
- Deferred rendering
- reflective shadow maps (RSM) by Carsten Dachsbacher and Marc Stamminger.
- проблема GLM_FORCE_LEFT_HANDED - исправить
*/
//=============================================================================
namespace
{
	struct ObjectUniforms
	{
		glm::mat4 model;
		glm::vec4 color;
	};

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	struct GlobalUniforms
	{
		glm::mat4 viewProj;
		glm::mat4 oldViewProj;
		glm::mat4 invViewProj;
		glm::mat4 proj;
		glm::vec4 cameraPos;
	};

	struct ShadingUniforms
	{
		glm::mat4 sunViewProj;
		glm::vec4 sunDir;
		glm::vec4 sunStrength;
	};

	struct RSMUniforms
	{
		glm::mat4 sunViewProj;
		glm::mat4 invSunViewProj;
		glm::ivec2 targetDim;
		float rMax;
		uint32_t currentPass;
		uint32_t samples;
		float random;
	};

	static constexpr auto gCubeVertices = std::array<Vertex, 24>{
		// front (+z)
		Vertex{{-0.5, -0.5, 0.5}, {0, 0, 1}, {0, 0}},
		{{0.5, -0.5, 0.5}, {0, 0, 1}, {1, 0}},
		{{0.5, 0.5, 0.5}, {0, 0, 1}, {1, 1}},
		{{-0.5, 0.5, 0.5}, {0, 0, 1}, {0, 1}},

		// back (-z)
		{{-0.5, 0.5, -0.5}, {0, 0, -1}, {1, 1}},
		{{0.5, 0.5, -0.5}, {0, 0, -1}, {0, 1}},
		{{0.5, -0.5, -0.5}, {0, 0, -1}, {0, 0}},
		{{-0.5, -0.5, -0.5}, {0, 0, -1}, {1, 0}},

		// left (-x)
		{{-0.5, -0.5, -0.5}, {-1, 0, 0}, {0, 0}},
		{{-0.5, -0.5, 0.5}, {-1, 0, 0}, {1, 0}},
		{{-0.5, 0.5, 0.5}, {-1, 0, 0}, {1, 1}},
		{{-0.5, 0.5, -0.5}, {-1, 0, 0}, {0, 1}},

		// right (+x)
		{{0.5, 0.5, -0.5}, {1, 0, 0}, {1, 1}},
		{{0.5, 0.5, 0.5}, {1, 0, 0}, {0, 1}},
		{{0.5, -0.5, 0.5}, {1, 0, 0}, {0, 0}},
		{{0.5, -0.5, -0.5}, {1, 0, 0}, {1, 0}},

		// top (+y)
		{{-0.5, 0.5, 0.5}, {0, 1, 0}, {0, 0}},
		{{0.5, 0.5, 0.5}, {0, 1, 0}, {1, 0}},
		{{0.5, 0.5, -0.5}, {0, 1, 0}, {1, 1}},
		{{-0.5, 0.5, -0.5}, {0, 1, 0}, {0, 1}},

		// bottom (-y)
		{{-0.5, -0.5, -0.5}, {0, -1, 0}, {0, 0}},
		{{0.5, -0.5, -0.5}, {0, -1, 0}, {1, 0}},
		{{0.5, -0.5, 0.5}, {0, -1, 0}, {1, 1}},
		{{-0.5, -0.5, 0.5}, {0, -1, 0}, {0, 1}},
	};

	static constexpr auto gCubeIndices = std::array<uint16_t, 36>{
	  0,  1,  2,  2,  3,  0,

	  4,  5,  6,  6,  7,  4,

	  8,  9,  10, 10, 11, 8,

	  12, 13, 14, 14, 15, 12,

	  16, 17, 18, 18, 19, 16,

	  20, 21, 22, 22, 23, 20,
	};


	static constexpr auto sceneInputBindingDescs = std::array{
	  gl4::VertexInputBindingDescription{
			// position
			.location = 0,
			.binding = 0,
			.format = gl4::Format::R32G32B32_FLOAT,
			.offset = offsetof(Vertex, position),
		  },
		  gl4::VertexInputBindingDescription{
			// normal
			.location = 1,
			.binding = 0,
			.format = gl4::Format::R32G32B32_FLOAT,
			.offset = offsetof(Vertex, normal),
		  },
		  gl4::VertexInputBindingDescription{
			// texcoord
			.location = 2,
			.binding = 0,
			.format = gl4::Format::R32G32_FLOAT,
			.offset = offsetof(Vertex, uv),
		  },
	};

	gl4::GraphicsPipeline CreateScenePipeline()
	{
		auto vs = gl4::Shader(gl4::PipelineStage::VertexShader, io::LoadFile("ExampleData/shaders/NewTest004/SceneDeferred.vert.glsl"));
		auto fs = gl4::Shader(gl4::PipelineStage::FragmentShader, io::LoadFile("ExampleData/shaders/NewTest004/SceneDeferred.frag.glsl"));

		return gl4::GraphicsPipeline({
		  .vertexShader = &vs,
		  .fragmentShader = &fs,
		  .vertexInputState = {sceneInputBindingDescs},
		  .depthState = {.depthTestEnable = true, .depthWriteEnable = true},
			});
	}

	gl4::GraphicsPipeline CreateShadowPipeline()
	{
		auto vs = gl4::Shader(gl4::PipelineStage::VertexShader, io::LoadFile("ExampleData/shaders/NewTest004/SceneDeferred.vert.glsl"));
		auto fs = gl4::Shader(gl4::PipelineStage::FragmentShader, io::LoadFile("ExampleData/shaders/NewTest004/RSMScene.frag.glsl"));

		return gl4::GraphicsPipeline({
		  .vertexShader = &vs,
		  .fragmentShader = &fs,
		  .vertexInputState = {sceneInputBindingDescs},
		  .depthState = {.depthTestEnable = true, .depthWriteEnable = true},
			});
	}

	gl4::GraphicsPipeline CreateShadingPipeline()
	{
		auto vs = gl4::Shader(gl4::PipelineStage::VertexShader, io::LoadFile("ExampleData/shaders/NewTest004/FullScreenTri.vert.glsl"));
		auto fs = gl4::Shader(gl4::PipelineStage::FragmentShader, io::LoadFile("ExampleData/shaders/NewTest004/ShadeDeferred.frag.glsl"));

		return gl4::GraphicsPipeline({
		  .vertexShader = &vs,
		  .fragmentShader = &fs,
		  .rasterizationState = {.cullMode = gl4::CullMode::None},
			});
	}

	gl4::GraphicsPipeline CreateDebugTexturePipeline()
	{
		auto vs = gl4::Shader(gl4::PipelineStage::VertexShader, io::LoadFile("ExampleData/shaders/NewTest004/FullScreenTri.vert.glsl"));
		auto fs = gl4::Shader(gl4::PipelineStage::FragmentShader, io::LoadFile("ExampleData/shaders/NewTest004/Texture.frag.glsl"));

		return gl4::GraphicsPipeline({
		  .vertexShader = &vs,
		  .fragmentShader = &fs,
		  .rasterizationState = {.cullMode = gl4::CullMode::None},
			});
	}


	// constants
	static constexpr int gShadowmapWidth = 1024;
	static constexpr int gShadowmapHeight = 1024;

	// scene parameters
	float sunPosition = 0;
	float sunPosition2 = 0;

	// transient variables
	double illuminationTime = 0;
	uint32_t sceneInstanceCount = 0;

	// Resources tied to the swapchain/output size
	struct Frame
	{
		// g-buffer textures
		std::optional<gl4::Texture> gAlbedo;
		std::optional<gl4::Texture> gNormal;
		std::optional<gl4::Texture> gDepth;
		std::optional<gl4::Texture> gNormalPrev;
		std::optional<gl4::Texture> gDepthPrev;
		std::optional<gl4::Texture> gMotion;
		std::optional<RSM::RsmTechnique> rsm;

		// For debug drawing with ImGui
		std::optional<gl4::TextureView> gAlbedoSwizzled;
		std::optional<gl4::TextureView> gNormalSwizzled;
		std::optional<gl4::TextureView> gDepthSwizzled;
		std::optional<gl4::TextureView> gRsmIlluminanceSwizzled;
	};
	Frame frame{};

	// Buffers describing the scene's objects and geometry
	std::optional<gl4::Buffer> vertexBuffer;
	std::optional<gl4::Buffer> indexBuffer;
	std::optional<gl4::Buffer> objectBuffer;

	// Reflective shadow map textures
	std::optional<gl4::Texture> rsmFlux;
	std::optional<gl4::Texture> rsmNormal;
	std::optional<gl4::Texture> rsmDepth;

	// For debug drawing with ImGui
	std::optional<gl4::TextureView> rsmFluxSwizzled;
	std::optional<gl4::TextureView> rsmNormalSwizzled;
	std::optional<gl4::TextureView> rsmDepthSwizzled;

	ShadingUniforms shadingUniforms;
	GlobalUniforms globalUniforms{};
	uint64_t frameIndex = 0;

	std::optional<gl4::TypedBuffer<GlobalUniforms>> globalUniformsBuffer;
	std::optional<gl4::TypedBuffer<ShadingUniforms>> shadingUniformsBuffer;

	std::optional<gl4::GraphicsPipeline> scenePipeline;
	std::optional<gl4::GraphicsPipeline> rsmScenePipeline;
	std::optional<gl4::GraphicsPipeline> shadingPipeline;
	std::optional<gl4::GraphicsPipeline> debugTexturePipeline;

	Camera mainCamera;


	void resize(uint16_t width, uint16_t height)
	{
		// create gbuffer textures and render info
		frame.gAlbedo = gl4::CreateTexture2D({ width, height }, gl4::Format::R8G8B8A8_SRGB);
		frame.gNormal = gl4::CreateTexture2D({ width, height }, gl4::Format::R16G16B16_SNORM);
		frame.gDepth = gl4::CreateTexture2D({ width, height }, gl4::Format::D32_UNORM);
		frame.gNormalPrev = gl4::CreateTexture2D({ width, height }, gl4::Format::R16G16B16_SNORM);
		frame.gDepthPrev = gl4::CreateTexture2D({ width, height }, gl4::Format::D32_UNORM);
		frame.gMotion = gl4::CreateTexture2D({ width, height }, gl4::Format::R16G16_FLOAT);

		frame.rsm = RSM::RsmTechnique(width, height);

		// create debug views
		frame.gAlbedoSwizzled = frame.gAlbedo->CreateSwizzleView({ .a = gl4::ComponentSwizzle::ONE });
		frame.gNormalSwizzled = frame.gNormal->CreateSwizzleView({ .a = gl4::ComponentSwizzle::ONE });
		frame.gDepthSwizzled = frame.gDepth->CreateSwizzleView({ .a = gl4::ComponentSwizzle::ONE });
		frame.gRsmIlluminanceSwizzled = frame.rsm->GetIndirectLighting().CreateSwizzleView({ .a = gl4::ComponentSwizzle::ONE });
	}
}
//=============================================================================
EngineCreateInfo NewTest004::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool NewTest004::OnInit()
{
	// Create RSM textures
	rsmFlux = gl4::CreateTexture2D({ gShadowmapWidth, gShadowmapHeight }, gl4::Format::R11G11B10_FLOAT);
	rsmNormal = gl4::CreateTexture2D({ gShadowmapWidth, gShadowmapHeight }, gl4::Format::R16G16B16_SNORM);
	rsmDepth = gl4::CreateTexture2D({ gShadowmapWidth, gShadowmapHeight }, gl4::Format::D16_UNORM);
	rsmFluxSwizzled = rsmFlux->CreateSwizzleView({ .a = gl4::ComponentSwizzle::ONE });
	rsmNormalSwizzled = rsmNormal->CreateSwizzleView({ .a = gl4::ComponentSwizzle::ONE });
	rsmDepthSwizzled = rsmDepth->CreateSwizzleView({ .a = gl4::ComponentSwizzle::ONE });
	// Create constant-size buffers
	globalUniformsBuffer = gl4::TypedBuffer<GlobalUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	shadingUniformsBuffer = gl4::TypedBuffer<ShadingUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	// Create the pipelines used in the application
	scenePipeline = CreateScenePipeline();
	rsmScenePipeline = CreateShadowPipeline();
	shadingPipeline = CreateShadingPipeline();
	debugTexturePipeline = CreateDebugTexturePipeline();

	ImGui::GetIO().Fonts->AddFontFromFileTTF("ExampleData/fonts/RobotoCondensed-Regular.ttf", 18);

	std::vector<ObjectUniforms> objectUniforms;
	// translation, scale, color tuples
	std::tuple<glm::vec3, glm::vec3, glm::vec3> objects[]{
	  {{0, .5, -1}, {3, 1, 1}, {.5, .5, .5}},
	  {{-1, .5, 0}, {1, 1, 1}, {.1, .1, .9}},
	  {{1, .5, 0}, {1, 1, 1}, {.1, .1, .9}},
	  {{0, -.5, -.5}, {3, 1, 2}, {.5, .5, .5}},
	  {{0, 1.5, -.5}, {3, 1, 2}, {.2, .7, .2}},
	  {{0, .25, 0}, {0.25, .5, .25}, {.5, .1, .1}},
	  //{ { -.25, .25, 0 },   { .01, .5, .7 }, { .5, .1, .1 } },
	  //{ { .25, .25, 0 },   { .01, .5, .7 }, { .5, .1, .1 } },
	  //{ { 0, .25, -.25 },   { .7, .5, .01 }, { .5, .1, .1 } },
	  //{ { 0, .25, .25 },   { .7, .5, .01 }, { .5, .1, .1 } },
	};
	for (const auto& [translation, scale, color] : objects)
	{
		glm::mat4 model{ 1 };
		model = glm::translate(model, translation);
		model = glm::scale(model, scale);
		objectUniforms.push_back({ model, glm::vec4{color, 0.0f} });
	}
	sceneInstanceCount = static_cast<uint32_t>(objectUniforms.size());

	vertexBuffer.emplace(gCubeVertices);
	indexBuffer.emplace(gCubeIndices);
	objectBuffer.emplace(std::span(objectUniforms), gl4::BufferStorageFlag::DynamicStorage);

	mainCamera.SetPosition(glm::vec3{ 0, .5, -1 });


	resize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void NewTest004::OnClose()
{
}
//=============================================================================
void NewTest004::OnUpdate(float deltaTime)
{
	frameIndex++;

	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_W) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(CameraForward, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_S) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(CameraBackward, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_A) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(CameraLeft, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_D) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(CameraRight, deltaTime);

	if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		SetCursorVisible(false);
		mainCamera.ProcessMouseMovement(-GetMouseDeltaX(), -GetMouseDeltaY());
	}
	else if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		glfwSetInputMode(GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		SetCursorVisible(true);
	}
}
//=============================================================================
void NewTest004::OnRender()
{
	std::swap(frame.gDepth, frame.gDepthPrev);
	std::swap(frame.gNormal, frame.gNormalPrev);

	shadingUniforms = ShadingUniforms{
	  .sunDir = glm::normalize(glm::rotate(sunPosition, glm::vec3{1, 0, 0}) *
							   glm::rotate(sunPosition2, glm::vec3(0, 1, 0)) * glm::vec4{-.1, -.3, -.6, 0}),
	  .sunStrength = glm::vec4{2, 2, 2, 0},
	};

	const auto proj = glm::perspective(glm::radians(70.f), GetWindowAspect(), 0.1f, 5.f);
	glm::mat4 viewProj = proj * mainCamera.GetViewMatrix();

	globalUniforms.oldViewProj = frameIndex == 1 ? viewProj : globalUniforms.viewProj;
	globalUniforms.proj = proj;
	globalUniforms.viewProj = viewProj;
	globalUniformsBuffer->UpdateData(globalUniforms);

	glm::vec3 eye = glm::vec3{ shadingUniforms.sunDir * -5.f };
	float eyeWidth = 2.5f;
	// shadingUniforms.viewPos = glm::vec4(camera.position, 0);
	auto projtemp = glm::ortho(-eyeWidth, eyeWidth, -eyeWidth, eyeWidth, .1f, 10.f);
	shadingUniforms.sunViewProj = projtemp * glm::lookAt(eye, glm::vec3(0), glm::vec3{ 0, 1, 0 });
	shadingUniformsBuffer->UpdateData(shadingUniforms);

	gl4::SamplerState ss;
	ss.minFilter = gl4::MinFilter::Nearest;
	ss.magFilter = gl4::MagFilter::Nearest;
	ss.addressModeU = gl4::AddressMode::Repeat;
	ss.addressModeV = gl4::AddressMode::Repeat;
	auto nearestSampler = gl4::Sampler(ss);

	// Render scene geometry to the g-buffer
	// DontCare indicates that the previous contents can be discarded before rendering
	auto gAlbedoAttachment = gl4::RenderColorAttachment{
	  .texture = frame.gAlbedo.value(),
	  .loadOp = gl4::AttachmentLoadOp::DontCare,
	};
	auto gNormalAttachment = gl4::RenderColorAttachment{
	  .texture = frame.gNormal.value(),
	  .loadOp = gl4::AttachmentLoadOp::DontCare,
	};
	auto gMotionAttachment = gl4::RenderColorAttachment{
	  .texture = frame.gMotion.value(),
	  .loadOp = gl4::AttachmentLoadOp::Clear,
	  .clearValue = glm::vec4{0.0f},
	};
	auto gDepthAttachment = gl4::RenderDepthStencilAttachment{
	  .texture = frame.gDepth.value(),
	  .loadOp = gl4::AttachmentLoadOp::Clear,
	  .clearValue = {.depth = 1.0f},
	};
	gl4::RenderColorAttachment cgAttachments[] = { gAlbedoAttachment, gNormalAttachment, gMotionAttachment };

	gl4::BeginRendering({
		  .name = "Base Pass",
		  .colorAttachments = cgAttachments,
		  .depthAttachment = gDepthAttachment,
		});
	{
		gl4::Cmd::BindGraphicsPipeline(scenePipeline.value());
		gl4::Cmd::BindVertexBuffer(0, *vertexBuffer, 0, sizeof(Vertex));
		gl4::Cmd::BindIndexBuffer(*indexBuffer, gl4::IndexType::UNSIGNED_SHORT);
		gl4::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
		gl4::Cmd::BindStorageBuffer(1, *objectBuffer);
		gl4::Cmd::DrawIndexed(static_cast<uint32_t>(gCubeIndices.size()), sceneInstanceCount, 0, 0, 0);
	}
	gl4::EndRendering();


	globalUniforms.viewProj = shadingUniforms.sunViewProj;
	globalUniformsBuffer->UpdateData(globalUniforms);

	// Shadow map (RSM) scene pass
	auto rcolorAttachment = gl4::RenderColorAttachment{
	  .texture = rsmFlux.value(),
	  .loadOp = gl4::AttachmentLoadOp::DontCare,
	};
	auto rnormalAttachment = gl4::RenderColorAttachment{
	  .texture = rsmNormal.value(),
	  .loadOp = gl4::AttachmentLoadOp::DontCare,
	};
	auto rdepthAttachment = gl4::RenderDepthStencilAttachment{
	  .texture = rsmDepth.value(),
	  .loadOp = gl4::AttachmentLoadOp::Clear,
	  .clearValue = {.depth = 1.0f},
	};
	gl4::RenderColorAttachment crAttachments[] = { rcolorAttachment, rnormalAttachment };

	gl4::BeginRendering({
		  .name = "RSM Scene",
		  .colorAttachments = crAttachments,
		  .depthAttachment = rdepthAttachment,
		});
	{
		gl4::Cmd::BindGraphicsPipeline(rsmScenePipeline.value());
		gl4::Cmd::BindVertexBuffer(0, *vertexBuffer, 0, sizeof(Vertex));
		gl4::Cmd::BindIndexBuffer(*indexBuffer, gl4::IndexType::UNSIGNED_SHORT);
		gl4::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
		gl4::Cmd::BindUniformBuffer(1, shadingUniformsBuffer.value());
		gl4::Cmd::BindStorageBuffer(1, *objectBuffer);
		gl4::Cmd::DrawIndexed(static_cast<uint32_t>(gCubeIndices.size()), sceneInstanceCount, 0, 0, 0);
	}
	gl4::EndRendering();

	globalUniforms.viewProj = viewProj;
	globalUniforms.invViewProj = glm::inverse(viewProj);
	globalUniformsBuffer->UpdateData(globalUniforms);

	{
		static gl4::TimerQueryAsync timer(5);
		if (auto t = timer.PopTimestamp())
		{
			illuminationTime = *t / 10e5;
		}
		gl4::TimerScoped scopedTimer(timer);

		auto rsmCameraUniforms = RSM::CameraUniforms{
		  .viewProj = viewProj,
		  .invViewProj = glm::inverse(viewProj),
		  .proj = proj,
		  .cameraPos = glm::vec4(mainCamera.Position, 0),
		  .viewDir = mainCamera.Front,
		};

		frame.rsm->ComputeIndirectLighting(shadingUniforms.sunViewProj,
			rsmCameraUniforms,
			frame.gAlbedo.value(),
			frame.gNormal.value(),
			frame.gDepth.value(),
			rsmFlux.value(),
			rsmNormal.value(),
			rsmDepth.value(),
			frame.gDepthPrev.value(),
			frame.gNormalPrev.value(),
			frame.gMotion.value());
	}

	// shading pass (full screen tri)
	gl4::BeginSwapChainRendering({
		  .name = "Shading",
		  .viewport =
			gl4::Viewport{
			  .drawRect{.offset = {0, 0}, .extent = {GetWindowWidth(), GetWindowHeight()}},
			  .minDepth = 0.0f,
			  .maxDepth = 1.0f,
			},
		  .colorLoadOp = gl4::AttachmentLoadOp::Clear,
		  .clearColorValue = {.1f, .3f, .5f, 0.0f},
		  .depthLoadOp = gl4::AttachmentLoadOp::DontCare,
		  .stencilLoadOp = gl4::AttachmentLoadOp::DontCare,
		});
	{
		gl4::Cmd::BindGraphicsPipeline(shadingPipeline.value());
		gl4::Cmd::BindSampledImage(0, *frame.gAlbedo, nearestSampler);
		gl4::Cmd::BindSampledImage(1, *frame.gNormal, nearestSampler);
		gl4::Cmd::BindSampledImage(2, *frame.gDepth, nearestSampler);
		gl4::Cmd::BindSampledImage(3, frame.rsm->GetIndirectLighting(), nearestSampler);
		gl4::Cmd::BindSampledImage(4, rsmDepth.value(), nearestSampler);
		gl4::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
		gl4::Cmd::BindUniformBuffer(1, shadingUniformsBuffer.value());
		gl4::Cmd::Draw(3, 1, 0, 0);

		const gl4::Texture* tex{};
		if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_F1) == GLFW_PRESS)
			tex = &frame.gAlbedo.value();
		if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_F2) == GLFW_PRESS)
			tex = &frame.gNormal.value();
		if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_F3) == GLFW_PRESS)
			tex = &frame.gDepth.value();
		if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_F4) == GLFW_PRESS)
			tex = &frame.rsm->GetIndirectLighting();
		if (tex)
		{
			gl4::Cmd::BindGraphicsPipeline(debugTexturePipeline.value());
			gl4::Cmd::BindSampledImage(0, *tex, nearestSampler);
			gl4::Cmd::Draw(3, 1, 0, 0);
		}
	}
	gl4::EndRendering();
}
//=============================================================================
void NewTest004::OnImGuiDraw()
{
	ImGui::Begin("Deferred");
	ImGui::Text("Framerate: %.0f Hertz", 1 / GetDeltaTime());
	ImGui::Text("Indirect Illumination: %f ms", illuminationTime);

	ImGui::SliderFloat("Sun Angle", &sunPosition, -1.2f, 2.1f);
	ImGui::SliderFloat("Sun Angle 2", &sunPosition2, -3.142f, 3.142f);

	ImGui::Separator();

	frame.rsm->DrawGui();

	ImGui::BeginTabBar("tabbed");
	if (ImGui::BeginTabItem("G-Buffers"))
	{
		float aspect = GetWindowAspect();
		ImGui::Image(static_cast<ImTextureID>(frame.gAlbedoSwizzled.value().Handle()),
			{ 100 * aspect, 100 },
			{ 0, 1 },
			{ 1, 0 });
		ImGui::SameLine();
		ImGui::Image(static_cast<ImTextureID>(frame.gNormalSwizzled.value().Handle()),
			{ 100 * aspect, 100 },
			{ 0, 1 },
			{ 1, 0 });
		ImGui::Image(static_cast<ImTextureID>(frame.gDepthSwizzled.value().Handle()),
			{ 100 * aspect, 100 },
			{ 0, 1 },
			{ 1, 0 });
		ImGui::SameLine();
		ImGui::Image(static_cast<ImTextureID>(frame.gRsmIlluminanceSwizzled.value().Handle()),
			{ 100 * aspect, 100 },
			{ 0, 1 },
			{ 1, 0 });
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("RSM Buffers"))
	{
		ImGui::Image(static_cast<ImTextureID>(rsmDepthSwizzled->Handle()), { 100, 100 }, { 0, 1 }, { 1, 0 });
		ImGui::SameLine();
		ImGui::Image(static_cast<ImTextureID>(rsmNormalSwizzled->Handle()),
			{ 100, 100 },
			{ 0, 1 },
			{ 1, 0 });
		ImGui::SameLine();
		ImGui::Image(static_cast<ImTextureID>(rsmFluxSwizzled->Handle()), { 100, 100 }, { 0, 1 }, { 1, 0 });
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();
	ImGui::End();

	DrawFPS();
}
//=============================================================================
void NewTest004::OnResize(uint16_t width, uint16_t height)
{
	resize(width, height);
}
//=============================================================================
void NewTest004::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void NewTest004::OnMousePos(double x, double y)
{
}
//=============================================================================
void NewTest004::OnScroll(double dx, double dy)
{
}
//=============================================================================
void NewTest004::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================