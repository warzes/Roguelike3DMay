#include "stdafx.h"
#include "NewTest005.h"
#include "RsmTechnique.h"
#include "SceneLoader.h"
// 03_gltf_viewer
//=============================================================================
namespace
{
	static glm::uint pcg_hash(glm::uint seed)
	{
		glm::uint state = seed * 747796405u + 2891336453u;
		glm::uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	// Used to advance the PCG state.
	static glm::uint rand_pcg(glm::uint& rng_state)
	{
		glm::uint state = rng_state;
		rng_state = rng_state * 747796405u + 2891336453u;
		glm::uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	// Advances the prng state and returns the corresponding random float.
	static float rng(glm::uint& state)
	{
		glm::uint x = rand_pcg(state);
		state = x;
		return float(x) * glm::uintBitsToFloat(0x2f800004u);
	}

	struct ObjectUniforms
	{
		glm::mat4 model;
	};

	struct GlobalUniforms
	{
		glm::mat4 viewProj;
		glm::mat4 oldViewProjUnjittered;
		glm::mat4 viewProjUnjittered;
		glm::mat4 invViewProj;
		glm::mat4 proj;
		glm::vec4 cameraPos;
	};

	struct ShadingUniforms
	{
		glm::mat4 sunViewProj;
		glm::vec4 sunDir;
		glm::vec4 sunStrength;
		glm::mat4 sunView;
		glm::mat4 sunProj;
		glm::vec2 random;
	};

	struct ShadowUniforms
	{
		uint32_t shadowMode = 0; // 0 = PCF, 1 = SMRT

		// PCF stuff
		uint32_t pcfSamples = 8;
		float pcfRadius = 0.002f;

		// SMRT stuff
		uint32_t shadowRays = 7;
		uint32_t stepsPerRay = 7;
		float rayStepSize = 0.1f;
		float heightmapThickness = 0.5f;
		float sourceAngleRad = 0.05f;
	};

	struct alignas(16) Light
	{
		glm::vec4 position;
		glm::vec3 intensity;
		float invRadius;
		// uint32_t type; // 0 = point, 1 = spot
	};

	static constexpr std::array<gl4::VertexInputBindingDescription, 3> sceneInputBindingDescs{
	  gl4::VertexInputBindingDescription{
		.location = 0,
		.binding = 0,
		.format = gl4::Format::R32G32B32_FLOAT,
		.offset = offsetof(Utility::Vertex, position),
	  },
	  gl4::VertexInputBindingDescription{
		.location = 1,
		.binding = 0,
		.format = gl4::Format::R16G16_SNORM,
		.offset = offsetof(Utility::Vertex, normal),
	  },
	  gl4::VertexInputBindingDescription{
		.location = 2,
		.binding = 0,
		.format = gl4::Format::R32G32_FLOAT,
		.offset = offsetof(Utility::Vertex, texcoord),
	  },
	};

	gl4::GraphicsPipeline CreateScenePipeline()
	{
		auto vs = gl4::Shader(gl4::PipelineStage::VertexShader, io::LoadFile("ExampleData/shaders/NewTest005/SceneDeferredPbr.vert.glsl"));
		auto fs = gl4::Shader(gl4::PipelineStage::FragmentShader, io::LoadFile("ExampleData/shaders/NewTest005/SceneDeferredPbr.frag.glsl"));

		return gl4::GraphicsPipeline({
		  .vertexShader = &vs,
		  .fragmentShader = &fs,
		  .vertexInputState = {sceneInputBindingDescs},
		  .depthState = {.depthTestEnable = true, .depthWriteEnable = true},
			});
	}

	gl4::GraphicsPipeline CreateShadowPipeline()
	{
		auto vs = gl4::Shader(gl4::PipelineStage::VertexShader, io::LoadFile("ExampleData/shaders/NewTest005/SceneDeferredPbr.vert.glsl"));
		auto fs = gl4::Shader(gl4::PipelineStage::FragmentShader, io::LoadFile("ExampleData/shaders/NewTest005/RSMScenePbr.frag.glsl"));

		return gl4::GraphicsPipeline({
		  .vertexShader = &vs,
		  .fragmentShader = &fs,
		  .vertexInputState = {sceneInputBindingDescs},
		  .depthState = {.depthTestEnable = true, .depthWriteEnable = true},
			});
	}

	gl4::GraphicsPipeline CreateShadingPipeline()
	{
		auto vs = gl4::Shader(gl4::PipelineStage::VertexShader, io::LoadFile("ExampleData/shaders/NewTest005/FullScreenTri.vert.glsl"));
		auto fs = gl4::Shader(gl4::PipelineStage::FragmentShader, io::LoadFile("ExampleData/shaders/NewTest005/ShadeDeferredPbr.frag.glsl"));

		return gl4::GraphicsPipeline({
		  .vertexShader = &vs,
		  .fragmentShader = &fs,
		  .rasterizationState = {.cullMode = gl4::CullMode::None},
			});
	}

	gl4::GraphicsPipeline CreatePostprocessingPipeline()
	{
		auto vs = gl4::Shader(gl4::PipelineStage::VertexShader, io::LoadFile("ExampleData/shaders/NewTest005/FullScreenTri.vert.glsl"));
		auto fs = gl4::Shader(gl4::PipelineStage::FragmentShader, io::LoadFile("ExampleData/shaders/NewTest005/TonemapAndDither.frag.glsl"));
		return gl4::GraphicsPipeline({
		  .vertexShader = &vs,
		  .fragmentShader = &fs,
		  .rasterizationState = {.cullMode = gl4::CullMode::None},
			});
	}

	gl4::GraphicsPipeline CreateDebugTexturePipeline()
	{
		auto vs = gl4::Shader(gl4::PipelineStage::VertexShader, io::LoadFile("ExampleData/shaders/NewTest005/FullScreenTri.vert.glsl"));
		auto fs = gl4::Shader(gl4::PipelineStage::FragmentShader, io::LoadFile("ExampleData/shaders/NewTest005/Texture.frag.glsl"));

		return gl4::GraphicsPipeline({
		  .vertexShader = &vs,
		  .fragmentShader = &fs,
		  .rasterizationState = {.cullMode = gl4::CullMode::None},
			});
	}


	// constants
	static constexpr int gShadowmapWidth = 2048;
	static constexpr int gShadowmapHeight = 2048;

	double illuminationTime = 0;
	double fsr2Time = 0;

	// scene parameters
	float sunPosition = -1.127f;
	float sunPosition2 = 0;
	float sunStrength = 50;
	glm::vec3 sunColor = { 1, 1, 1 };

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
		std::optional<gl4::Texture> colorHdrRenderRes;
		std::optional<gl4::Texture> colorHdrWindowRes;
		std::optional<gl4::Texture> colorLdrWindowRes;
		std::optional<RSM::RsmTechnique> rsm;

		// For debug drawing with ImGui
		std::optional<gl4::TextureView> gAlbedoSwizzled;
		std::optional<gl4::TextureView> gNormalSwizzled;
		std::optional<gl4::TextureView> gDepthSwizzled;
		std::optional<gl4::TextureView> gRsmIlluminanceSwizzled;
	};
	Frame frame{};

	// Reflective shadow map textures
	std::optional<gl4::Texture> rsmFlux;
	std::optional<gl4::Texture> rsmNormal;
	std::optional<gl4::Texture> rsmDepth;

	// For debug drawing with ImGui
	std::optional<gl4::TextureView> rsmFluxSwizzled;
	std::optional<gl4::TextureView> rsmNormalSwizzled;
	std::optional<gl4::TextureView> rsmDepthSwizzled;

	ShadingUniforms shadingUniforms{};
	ShadowUniforms shadowUniforms{};
	GlobalUniforms mainCameraUniforms{};

	std::optional<gl4::TypedBuffer<GlobalUniforms>> globalUniformsBuffer;
	std::optional<gl4::TypedBuffer<ShadingUniforms>> shadingUniformsBuffer;
	std::optional<gl4::TypedBuffer<ShadowUniforms>> shadowUniformsBuffer;
	std::optional<gl4::TypedBuffer<Utility::GpuMaterial>> materialUniformsBuffer;
	std::optional<gl4::TypedBuffer<glm::mat4>> rsmUniforms;

	std::optional<gl4::GraphicsPipeline> scenePipeline;
	std::optional<gl4::GraphicsPipeline> rsmScenePipeline;
	std::optional<gl4::GraphicsPipeline> shadingPipeline;
	std::optional<gl4::GraphicsPipeline> postprocessingPipeline;
	std::optional<gl4::GraphicsPipeline> debugTexturePipeline;

	// Scene
	Utility::Scene scene;
	std::optional<gl4::TypedBuffer<Light>> lightBuffer;
	std::optional<gl4::TypedBuffer<ObjectUniforms>> meshUniformBuffer;

	// Post processing
	std::optional<gl4::Texture> noiseTexture;

	uint32_t renderWidth;
	uint32_t renderHeight;
	uint32_t frameIndex = 0;
	uint32_t seed = pcg_hash(17);

	// Magnifier
	bool magnifierLock = false;
	float magnifierScale = 0.0173f;

	Camera mainCamera;

	std::vector<ObjectUniforms> meshUniforms;
	std::vector<Light> lights;


	void resize(uint16_t width, uint16_t height)
	{
		renderWidth = width;
		renderHeight = height;

		// create gbuffer textures and render info
		frame.gAlbedo = gl4::CreateTexture2D({ renderWidth, renderHeight }, gl4::Format::R8G8B8A8_SRGB, "gAlbedo");
		frame.gNormal = gl4::CreateTexture2D({ renderWidth, renderHeight }, gl4::Format::R16G16B16_SNORM, "gNormal");
		frame.gDepth = gl4::CreateTexture2D({ renderWidth, renderHeight }, gl4::Format::D32_FLOAT, "gDepth");
		frame.gNormalPrev = gl4::CreateTexture2D({ renderWidth, renderHeight }, gl4::Format::R16G16B16_SNORM);
		frame.gDepthPrev = gl4::CreateTexture2D({ renderWidth, renderHeight }, gl4::Format::D32_FLOAT);
		frame.gMotion = gl4::CreateTexture2D({ renderWidth, renderHeight }, gl4::Format::R16G16_FLOAT, "gMotion");
		frame.colorHdrRenderRes =
			gl4::CreateTexture2D({ renderWidth, renderHeight }, gl4::Format::R11G11B10_FLOAT, "colorHdrRenderRes");
		frame.colorHdrWindowRes = gl4::CreateTexture2D({ width, height }, gl4::Format::R11G11B10_FLOAT, "colorHdrWindowRes");
		frame.colorLdrWindowRes = gl4::CreateTexture2D({ width, height }, gl4::Format::R8G8B8A8_UNORM, "colorLdrWindowRes");

		if (!frame.rsm)
		{
			frame.rsm = RSM::RsmTechnique(renderWidth, renderHeight);
		}
		else
		{
			frame.rsm->SetResolution(renderWidth, renderHeight);
		}

		// create debug views
		frame.gAlbedoSwizzled = frame.gAlbedo->CreateSwizzleView({ .a = gl4::ComponentSwizzle::ONE });
		frame.gNormalSwizzled = frame.gNormal->CreateSwizzleView({ .a = gl4::ComponentSwizzle::ONE });
		frame.gDepthSwizzled = frame.gDepth->CreateSwizzleView({ .a = gl4::ComponentSwizzle::ONE });
		frame.gRsmIlluminanceSwizzled = frame.rsm->GetIndirectLighting().CreateSwizzleView({ .a = gl4::ComponentSwizzle::ONE });
	}
}
//=============================================================================
EngineCreateInfo NewTest005::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool NewTest005::OnInit()
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
	shadowUniformsBuffer = gl4::TypedBuffer<ShadowUniforms>(shadowUniforms, gl4::BufferStorageFlag::DynamicStorage);
	materialUniformsBuffer = gl4::TypedBuffer<Utility::GpuMaterial>(gl4::BufferStorageFlag::DynamicStorage);	
	rsmUniforms = gl4::TypedBuffer<glm::mat4>(gl4::BufferStorageFlag::DynamicStorage);
	// Create the pipelines used in the application
	scenePipeline = CreateScenePipeline();
	rsmScenePipeline = CreateShadowPipeline();
	shadingPipeline = CreateShadingPipeline();
	postprocessingPipeline = CreatePostprocessingPipeline();
	debugTexturePipeline = CreateDebugTexturePipeline();

	int x = 0;
	int y = 0;
	const auto noise = stbi_load("CoreData/textures/bluenoise32.png", &x, &y, nullptr, 4);
	assert(noise);
	noiseTexture = gl4::CreateTexture2D({ static_cast<uint32_t>(x), static_cast<uint32_t>(y) }, gl4::Format::R8G8B8A8_UNORM);
	noiseTexture->UpdateImage({
	  .extent = {static_cast<uint32_t>(x), static_cast<uint32_t>(y)},
	  .format = gl4::UploadFormat::RGBA,
	  .type = gl4::UploadType::UBYTE,
	  .pixels = noise,
		});
	stbi_image_free(noise);

	ImGui::GetIO().Fonts->AddFontFromFileTTF("ExampleData/fonts/RobotoCondensed-Regular.ttf", 18);

	Utility::LoadModelFromFile(scene, "ExampleData/mesh/simple_scene.glb", glm::mat4{ .125 }, true);

	for (size_t i = 0; i < scene.meshes.size(); i++)
	{
		meshUniforms.push_back({ scene.meshes[i].transform });
	}
	// lights.push_back(Light{ .position = { 3, 2, 0, 0 }, .intensity = { .2f, .8f, 1.0f }, .invRadius = 1.0f / 4.0f });
  // lights.push_back(Light{ .position = { 3, -2, 0, 0 }, .intensity = { .7f, .8f, 0.1f }, .invRadius = 1.0f / 2.0f });
  // lights.push_back(Light{ .position = { 3, 2, 0, 0 }, .intensity = { 1.2f, .8f, .1f }, .invRadius = 1.0f / 6.0f });

	meshUniformBuffer.emplace(meshUniforms, gl4::BufferStorageFlag::DynamicStorage);

	if (!lights.empty())
	{
		lightBuffer.emplace(lights, gl4::BufferStorageFlag::DynamicStorage);
	}

	// clusterTexture({.imageType = gl4::ImageType::TEX_3D,
  //                                      .format = gl4::Format::R16G16_UINT,
  //                                      .extent = {16, 9, 24},
  //                                      .mipLevels = 1,
  //                                      .arrayLayers = 1,
  //                                      .sampleCount = gl4::SampleCount::SAMPLES_1},
  //                                     "Cluster Texture");

  //// atomic counter + uint array
  // clusterIndicesBuffer = gl4::Buffer(sizeof(uint32_t) + sizeof(uint32_t) * 10000);
  // const uint32_t zero = 0; // what it says on the tin
  // clusterIndicesBuffer.ClearSubData(0,
  //                                   clusterIndicesBuffer.Size(),
  //                                   gl4::Format::R32_UINT,
  //                                   gl4::UploadFormat::R,
  //                                   gl4::UploadType::UINT,
  //                                   &zero);

	mainCamera.SetPosition(glm::vec3{ 0, .5, -1 });


	resize(GetWindowWidth(), GetWindowHeight());

	return true;
}
//=============================================================================
void NewTest005::OnClose()
{
}
//=============================================================================
void NewTest005::OnUpdate(float deltaTime)
{
	shadingUniforms.random = { 0, 0 };

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
void NewTest005::OnRender()
{
	std::swap(frame.gDepth, frame.gDepthPrev);
	std::swap(frame.gNormal, frame.gNormalPrev);

	shadingUniforms.sunDir = glm::normalize(glm::rotate(sunPosition, glm::vec3{ 1, 0, 0 }) *
		glm::rotate(sunPosition2, glm::vec3(0, 1, 0)) * glm::vec4{ -.1, -.3, -.6, 0 });
	shadingUniforms.sunStrength = glm::vec4{ sunStrength * sunColor, 0 };

	const float fsr2LodBias = 0;

	gl4::SamplerState ss;

	ss.minFilter = gl4::MinFilter::Nearest;
	ss.magFilter = gl4::MagFilter::Nearest;
	ss.addressModeU = gl4::AddressMode::Repeat;
	ss.addressModeV = gl4::AddressMode::Repeat;
	auto nearestSampler = gl4::Sampler(ss);

	ss.lodBias = 0;
	ss.compareEnable = true;
	ss.compareOp = gl4::CompareOp::Less;
	auto shadowSampler = gl4::Sampler(ss);

	constexpr float cameraNear = 0.1f;
	constexpr float cameraFar = 100.0f;
	constexpr float cameraFovY = glm::radians(70.f);
	const auto jitterOffset = glm::vec2{};
	const auto jitterMatrix = glm::translate(glm::mat4(1), glm::vec3(jitterOffset, 0));
	const auto projUnjittered = glm::perspectiveNO(cameraFovY, renderWidth / (float)renderHeight, cameraNear, cameraFar);
	const auto projJittered = jitterMatrix * projUnjittered;

	// Set global uniforms
	const auto viewProj = projJittered * mainCamera.GetViewMatrix();
	const auto viewProjUnjittered = projUnjittered * mainCamera.GetViewMatrix();
	mainCameraUniforms.oldViewProjUnjittered = frameIndex == 1 ? viewProjUnjittered : mainCameraUniforms.viewProjUnjittered;
	mainCameraUniforms.viewProjUnjittered = viewProjUnjittered;
	mainCameraUniforms.viewProj = viewProj;
	mainCameraUniforms.invViewProj = glm::inverse(mainCameraUniforms.viewProj);
	mainCameraUniforms.proj = projJittered;
	mainCameraUniforms.cameraPos = glm::vec4(mainCamera.Position, 0.0);

	globalUniformsBuffer->UpdateData(mainCameraUniforms);

	shadowUniformsBuffer->UpdateData(shadowUniforms);

	glm::vec3 eye = glm::vec3{ shadingUniforms.sunDir * -5.f };
	float eyeWidth = 7.0f;
	// shadingUniforms.viewPos = glm::vec4(camera.position, 0);
	shadingUniforms.sunProj = glm::orthoZO(-eyeWidth, eyeWidth, -eyeWidth, eyeWidth, -100.0f, 100.f);
	shadingUniforms.sunView = glm::lookAt(eye, glm::vec3(0), glm::vec3{ 0, 1, 0 });
	shadingUniforms.sunViewProj = shadingUniforms.sunProj * shadingUniforms.sunView;
	shadingUniformsBuffer->UpdateData(shadingUniforms);

	// Render scene geometry to the g-buffer
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
	  .clearValue = {0.f, 0.f, 0.f, 0.f},
	};
	auto gDepthAttachment = gl4::RenderDepthStencilAttachment{
	  .texture = frame.gDepth.value(),
	  .loadOp = gl4::AttachmentLoadOp::Clear,
	  .clearValue = {.depth = 1.0f},
	};
	gl4::RenderColorAttachment cgAttachments[] = { gAlbedoAttachment, gNormalAttachment, gMotionAttachment };

	gl4::BeginRendering({
			.name = "Base Pass",
			.viewport =
			gl4::Viewport{
			  .drawRect = {{0, 0}, {renderWidth, renderHeight}},
			  .depthRange = gl4::ClipDepthRange::NEGATIVE_ONE_TO_ONE,
			},
		  .colorAttachments = cgAttachments,
		  .depthAttachment = gDepthAttachment,
		});
	{
		gl4::Cmd::BindGraphicsPipeline(scenePipeline.value());
		gl4::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
		gl4::Cmd::BindUniformBuffer(2, materialUniformsBuffer.value());

		gl4::Cmd::BindStorageBuffer(1, *meshUniformBuffer);
		for (uint32_t i = 0; i < static_cast<uint32_t>(scene.meshes.size()); i++)
		{
			const auto& mesh = scene.meshes[i];
			const auto& material = scene.materials[mesh.materialIdx];
			materialUniformsBuffer->UpdateData(material.gpuMaterial);
			if (material.gpuMaterial.flags & Utility::MaterialFlagBit::HAS_BASE_COLOR_TEXTURE)
			{
				const auto& textureSampler = material.albedoTextureSampler.value();
				auto sampler = textureSampler.sampler;
				sampler.lodBias = fsr2LodBias;
				gl4::Cmd::BindSampledImage(0, textureSampler.texture, gl4::Sampler(sampler));
			}
			gl4::Cmd::BindVertexBuffer(0, mesh.vertexBuffer, 0, sizeof(Utility::Vertex));
			gl4::Cmd::BindIndexBuffer(mesh.indexBuffer, gl4::IndexType::UNSIGNED_INT);
			gl4::Cmd::DrawIndexed(static_cast<uint32_t>(mesh.indexBuffer.Size()) / sizeof(uint32_t), 1, 0, 0, i);
		}
	}
	gl4::EndRendering();

	rsmUniforms->UpdateData(shadingUniforms.sunViewProj);

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
		gl4::Cmd::BindUniformBuffer(0, rsmUniforms.value());
		gl4::Cmd::BindUniformBuffer(1, shadingUniformsBuffer.value());
		gl4::Cmd::BindUniformBuffer(2, materialUniformsBuffer.value());

		gl4::Cmd::BindStorageBuffer(1, *meshUniformBuffer, 0);
		for (uint32_t i = 0; i < static_cast<uint32_t>(scene.meshes.size()); i++)
		{
			const auto& mesh = scene.meshes[i];
			const auto& material = scene.materials[mesh.materialIdx];
			materialUniformsBuffer->UpdateData(material.gpuMaterial);
			if (material.gpuMaterial.flags & Utility::MaterialFlagBit::HAS_BASE_COLOR_TEXTURE)
			{
				const auto& textureSampler = material.albedoTextureSampler.value();
				gl4::Cmd::BindSampledImage(0, textureSampler.texture, gl4::Sampler(textureSampler.sampler));
			}
			gl4::Cmd::BindVertexBuffer(0, mesh.vertexBuffer, 0, sizeof(Utility::Vertex));
			gl4::Cmd::BindIndexBuffer(mesh.indexBuffer, gl4::IndexType::UNSIGNED_INT);
			gl4::Cmd::DrawIndexed(static_cast<uint32_t>(mesh.indexBuffer.Size()) / sizeof(uint32_t), 1, 0, 0, i);
		}
	}
	gl4::EndRendering();


	auto rsmCameraUniforms = RSM::CameraUniforms{
	  .viewProj = projUnjittered * mainCamera.GetViewMatrix(),
	  .invViewProj = glm::inverse(viewProjUnjittered),
	  .proj = projUnjittered,
	  .cameraPos = glm::vec4(mainCamera.Position, 0),
	  .viewDir = mainCamera.Front,
	  .jitterOffset = jitterOffset,
	  .lastFrameJitterOffset = glm::vec2{},
	};

	{
		static gl4::TimerQueryAsync timer(5);
		if (auto t = timer.PopTimestamp())
		{
			illuminationTime = *t / 10e5;
		}
		gl4::TimerScoped scopedTimer(timer);
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

	// clear cluster indices atomic counter
	// clusterIndicesBuffer.ClearSubData(0, sizeof(uint32_t), gl4::Format::R32_UINT, gl4::UploadFormat::R, gl4::UploadType::UINT, &zero);

	// record active clusters
	// TODO

	// light culling+cluster assignment

	//

	// shading pass (full screen tri)

	auto shadingColorAttachment = gl4::RenderColorAttachment{
	  .texture = frame.colorHdrRenderRes.value(),
	  .loadOp = gl4::AttachmentLoadOp::Clear,
	  .clearValue = {.1f, .3f, .5f, 0.0f},
	};
	gl4::BeginRendering({
	  .name = "Shading",
		  .colorAttachments = {&shadingColorAttachment, 1},
		});
	{
		gl4::Cmd::BindGraphicsPipeline(shadingPipeline.value());
		gl4::Cmd::BindSampledImage(0, *frame.gAlbedo, nearestSampler);
		gl4::Cmd::BindSampledImage(1, *frame.gNormal, nearestSampler);
		gl4::Cmd::BindSampledImage(2, *frame.gDepth, nearestSampler);
		gl4::Cmd::BindSampledImage(3, frame.rsm->GetIndirectLighting(), nearestSampler);
		gl4::Cmd::BindSampledImage(4, rsmDepth.value(), nearestSampler);
		gl4::Cmd::BindSampledImage(5, rsmDepth.value(), shadowSampler);
		gl4::Cmd::BindUniformBuffer(0, globalUniformsBuffer.value());
		gl4::Cmd::BindUniformBuffer(1, shadingUniformsBuffer.value());
		gl4::Cmd::BindUniformBuffer(2, shadowUniformsBuffer.value());
		if (lightBuffer)
		{
			gl4::Cmd::BindStorageBuffer(0, *lightBuffer);
		}
		gl4::Cmd::Draw(3, 1, 0, 0);
	}
	gl4::EndRendering();

	const auto ppAttachment = gl4::RenderColorAttachment{
	  .texture = frame.colorLdrWindowRes.value(),
	  .loadOp = gl4::AttachmentLoadOp::DontCare,
	};

	gl4::BeginRendering({
  .name = "Postprocessing",
		  .colorAttachments = {&ppAttachment, 1},
		});
	{
		gl4::Cmd::BindGraphicsPipeline(postprocessingPipeline.value());
		gl4::Cmd::BindSampledImage(0, frame.colorHdrRenderRes.value(), nearestSampler);
		gl4::Cmd::BindSampledImage(1, noiseTexture.value(), nearestSampler);
		gl4::Cmd::Draw(3, 1, 0, 0);
	}
	gl4::EndRendering();

	gl4::BeginSwapChainRendering({
		   .name = "Copy to swapchain",
		  .viewport =
			gl4::Viewport{
			  .drawRect{.offset = {0, 0}, .extent = {GetWindowWidth(), GetWindowHeight()}},
			  .minDepth = 0.0f,
			  .maxDepth = 1.0f,
			},
		  .colorLoadOp = gl4::AttachmentLoadOp::DontCare,
		  .depthLoadOp = gl4::AttachmentLoadOp::DontCare,
		  .stencilLoadOp = gl4::AttachmentLoadOp::DontCare,
		  .enableSrgb = false,
		});
	{
		const gl4::Texture* tex = &frame.colorLdrWindowRes.value();
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
void NewTest005::OnImGuiDraw()
{
	ImGui::Begin("glTF Viewer");
	ImGui::Text("Framerate: %.0f Hertz", 1 / GetDeltaTime());
	ImGui::Text("Indirect Illumination: %f ms", illuminationTime);
	ImGui::Text("FSR 2: %f ms", fsr2Time);

	ImGui::SliderFloat("Sun Angle", &sunPosition, -2.7f, 0.5f);
	ImGui::SliderFloat("Sun Angle 2", &sunPosition2, -3.142f, 3.142f);
	ImGui::ColorEdit3("Sun Color", &sunColor[0], ImGuiColorEditFlags_Float);
	ImGui::SliderFloat("Sun Strength", &sunStrength, 0, 50);

	ImGui::Separator();

	frame.rsm->DrawGui();

	ImGui::Separator();

	ImGui::Text("Shadow");

	auto SliderUint = [](const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max) -> bool
		{
			int tempv = static_cast<int>(*v);
			if (ImGui::SliderInt(label, &tempv, static_cast<int>(v_min), static_cast<int>(v_max)))
			{
				*v = static_cast<uint32_t>(tempv);
				return true;
			}
			return false;
		};

	int shadowMode = shadowUniforms.shadowMode;
	ImGui::RadioButton("PCF", &shadowMode, 0);
	ImGui::SameLine();
	ImGui::RadioButton("SMRT", &shadowMode, 1);
	shadowUniforms.shadowMode = shadowMode;

	if (shadowMode == 0)
	{
		SliderUint("PCF Samples", &shadowUniforms.pcfSamples, 1, 16);
		ImGui::SliderFloat("PCF Radius", &shadowUniforms.pcfRadius, 0, 0.01f, "%.4f");
	}
	else if (shadowMode == 1)
	{
		SliderUint("Shadow Rays", &shadowUniforms.shadowRays, 1, 10);
		SliderUint("Steps Per Ray", &shadowUniforms.stepsPerRay, 1, 20);
		ImGui::SliderFloat("Ray Step Size", &shadowUniforms.rayStepSize, 0.01f, 1.0f);
		ImGui::SliderFloat("Heightmap Thickness", &shadowUniforms.heightmapThickness, 0.05f, 1.0f);
		ImGui::SliderFloat("Light Spread", &shadowUniforms.sourceAngleRad, 0.001f, 0.3f);
	}

	ImGui::BeginTabBar("tabbed");
	if (ImGui::BeginTabItem("G-Buffers"))
	{
		float aspect = float(renderWidth) / renderHeight;
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

	/*ImGui::Begin(
		("Magnifier: " + std::string(magnifierLock ? "Locked (L, Space)" : "Unlocked (L, Space)") + "###mag").c_str());
	if (ImGui::GetKeyPressedAmount(ImGuiKey_KeypadSubtract, 10000, 1))
	{
		magnifierScale *= 1.5f;
	}
	if (ImGui::GetKeyPressedAmount(ImGuiKey_KeypadAdd, 10000, 1))
	{
		magnifierScale /= 1.5f;
	}
	float scale = 1.0f / magnifierScale;
	ImGui::SliderFloat("Scale (+, -)", &scale, 2.0f, 250.0f, "%.0f", ImGuiSliderFlags_Logarithmic);
	magnifierScale = 1.0f / scale;
	if (ImGui::GetKeyPressedAmount(ImGuiKey_L, 10000, 1) || ImGui::GetKeyPressedAmount(ImGuiKey_Space, 10000, 1))
	{
		magnifierLock = !magnifierLock;
	}
	double x{}, y{};
	glfwGetCursorPos(GetGLFWWindow(), &x, &y);
	glm::vec2 mp = magnifierLock ? GetCurs : glm::vec2{ x, y };
	lastCursorPos = mp;
	mp.y = windowHeight - mp.y;
	mp /= glm::vec2(windowWidth, windowHeight);
	float ar = (float)windowWidth / (float)windowHeight;
	glm::vec2 uv0{ mp.x - magnifierScale, mp.y + magnifierScale * ar };
	glm::vec2 uv1{ mp.x + magnifierScale, mp.y - magnifierScale * ar };
	uv0 = glm::clamp(uv0, glm::vec2(0), glm::vec2(1));
	uv1 = glm::clamp(uv1, glm::vec2(0), glm::vec2(1));
	glTextureParameteri(frame.colorLdrWindowRes.value().Handle(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(frame.colorLdrWindowRes.value().Handle())),
		ImVec2(400, 400),
		ImVec2(uv0.x, uv0.y),
		ImVec2(uv1.x, uv1.y));
	ImGui::End();*/

	DrawFPS();
}
//=============================================================================
void NewTest005::OnResize(uint16_t width, uint16_t height)
{
	resize(width, height);
}
//=============================================================================
void NewTest005::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void NewTest005::OnMousePos(double x, double y)
{
}
//=============================================================================
void NewTest005::OnScroll(double dx, double dy)
{
}
//=============================================================================
void NewTest005::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================