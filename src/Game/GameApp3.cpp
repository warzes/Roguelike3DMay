#include "stdafx.h"
#include "GameApp3.h"

//
//переделать загрузку меша на основе кода SceneLoader.h через fastgltf: нужна поддержка мультимеша, материалов, свойств модели.
//также возможно нужно чтобы оно брало fastgltf для gltf, тиниобж для обж, и асимп для остального
//
//обновить сторонние либы
//
//возможно отключить левостороннюю систему координат
//
//StratusGFX
//
//https://github.com/QwePek/LightingOpenGL

//=============================================================================
GameApp3::GameApp3()
{
}
//=============================================================================
EngineCreateInfo GameApp3::GetCreateInfo() const
{
	EngineCreateInfo createInfo{};
	createInfo.render.vsync = true;

	return createInfo;
}
//=============================================================================
bool GameApp3::OnInit()
{
	OnResize(GetWindowWidth(), GetWindowHeight());
		
	m_camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	std::vector<MeshVertex> vertices = {
		// positions                                // normals            // texcoords
		{{-50.0f, -0.25f,  50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  { 0.0f,  0.0f}},
		{{-50.0f, -0.25f, -50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  { 0.0f, 50.0f}},
		{{ 50.0f, -0.25f,  50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  {50.0f,  0.0f}},
		{{ 50.0f, -0.25f,  50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  {50.0f,  0.0f}},
		{{-50.0f, -0.25f, -50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  { 0.0f, 50.0f}},
		{{ 50.0f, -0.25f, -50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  {50.0f, 50.0f}}
	};
	std::vector<uint32_t> indices = { 0, 1, 2, 3, 4, 5 };

	m_model1.mesh = LoadDataMesh(vertices, indices);
	m_model1.textureFilter = gl4::MagFilter::Nearest;
	m_model1.material.diffuseTexture = TextureManager::GetTexture("ExampleData/textures/wood.jpg");
	m_model1.material.normalTexture = TextureManager::GetTexture("CoreData/textures/normal01.tga");

	m_model2.mesh = LoadAssimpMesh("ExampleData/mesh/bunny.obj");
	m_model2.scale = glm::vec3(3.0f);

	m_model3 = LoadAssimpModel("ExampleData/mesh/school/school.obj");
//	m_model3 = LoadAssimpModel("ExampleData/mesh/metro/metro.obj");


	if (!createPipeline())
		return false;

	m_globalUbo = gl4::TypedBuffer<GlobalUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_objectUbo = gl4::TypedBuffer<ObjectUniforms>(gl4::BufferStorageFlag::DynamicStorage);
	m_materialUbo = gl4::TypedBuffer<MaterialUniforms>(gl4::BufferStorageFlag::DynamicStorage);

	gl4::SamplerState sampleDesc;
	sampleDesc.minFilter = gl4::MinFilter::Nearest;
	sampleDesc.magFilter = gl4::MagFilter::Nearest;
	sampleDesc.addressModeU = gl4::AddressMode::Repeat;
	sampleDesc.addressModeV = gl4::AddressMode::Repeat;
	m_nearestSampler = gl4::Sampler(sampleDesc);

	sampleDesc.anisotropy = gl4::SampleCount::Samples16;
	sampleDesc.minFilter = gl4::MinFilter::LinearMimapLinear;
	sampleDesc.magFilter = gl4::MagFilter::Linear;
	sampleDesc.addressModeU = gl4::AddressMode::Repeat;
	sampleDesc.addressModeV = gl4::AddressMode::Repeat;
	m_linearSampler = gl4::Sampler(sampleDesc);

	return true;
}
//=============================================================================
void GameApp3::OnClose()
{
	delete m_model1.mesh;
	delete m_model2.mesh;

	m_nearestSampler = {};
	m_linearSampler = {};

	m_globalUbo = {};
	m_objectUbo = {};
	m_materialUbo = {};

	m_finalColorBuffer = {};
	m_finalDepthBuffer = {};
}
//=============================================================================
void GameApp3::OnUpdate(float deltaTime)
{
	if (Input::IsKeyDown(GLFW_KEY_W)) m_camera.ProcessKeyboard(CameraForward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_S)) m_camera.ProcessKeyboard(CameraBackward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_A)) m_camera.ProcessKeyboard(CameraLeft, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_D)) m_camera.ProcessKeyboard(CameraRight, deltaTime);

	if (Input::IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(false);
		m_camera.ProcessMouseMovement(-Input::GetScreenOffset().x, Input::GetScreenOffset().y);
	}
	else if (Input::IsMouseReleased(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(true);
	}
}
//=============================================================================
void GameApp3::OnRender()
{
	{
		auto finalColorAttachment = gl4::RenderColorAttachment{
			.texture = m_finalColorBuffer.value(),
			.loadOp = gl4::AttachmentLoadOp::Clear,
			.clearValue = { 0.1f, 0.5f, 0.8f, 1.0f },
		};
		auto finalDepthAttachment = gl4::RenderDepthStencilAttachment{
		  .texture = m_finalDepthBuffer.value(),
		  .loadOp = gl4::AttachmentLoadOp::Clear,
		  .clearValue = {.depth = 1.0f},
		};

		gl4::BeginRendering({ .colorAttachments = {&finalColorAttachment, 1}, .depthAttachment = finalDepthAttachment });
		{
			m_globalUboData.view = m_camera.GetViewMatrix();
			m_globalUboData.proj = m_projection;
			m_globalUboData.eyePosition = m_camera.Position;
			m_globalUbo->UpdateData(m_globalUboData);

			gl4::Cmd::BindGraphicsPipeline(m_pipeline.value());
			gl4::Cmd::BindUniformBuffer(0, m_globalUbo.value());

			drawModel(m_model1);
			drawModel(m_model2);
			drawModel(m_model3);

		}
		gl4::EndRendering();
	}
		
	//-------------------------------------------------------------------------
	// FINAL PASS
	//-------------------------------------------------------------------------
	gl4::BlitTextureToSwapchain(*m_finalColorBuffer, {}, {}, m_finalColorBuffer->Extent(), { GetWindowWidth(), GetWindowHeight(), 1 }, gl4::MagFilter::Nearest);
}
//=============================================================================
void GameApp3::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void GameApp3::OnResize(uint16_t width, uint16_t height)
{
	m_finalColorBuffer = gl4::CreateTexture2D({ width, height }, gl4::Format::R8G8B8A8_SRGB, "FinalColorBuffer");
	m_finalDepthBuffer = gl4::CreateTexture2D({ width, height }, gl4::Format::D32_FLOAT, "FinalDepthBuffer");

	m_projection = glm::perspective(glm::radians(65.0f), GetWindowAspect(), 0.01f, 1000.0f);
}
//=============================================================================
void GameApp3::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void GameApp3::OnMousePos(double x, double y)
{
}
//=============================================================================
void GameApp3::OnScroll(double dx, double dy)
{
}
//=============================================================================
void GameApp3::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================
void GameApp3::drawModel(GameModelOld& model)
{
	const gl4::Sampler& sampler = (model.textureFilter == gl4::MagFilter::Linear)
		? m_linearSampler.value()
		: m_nearestSampler.value();

	m_objectUboData.model = model.GetModelMat();
	m_objectUbo->UpdateData(m_objectUboData);
	gl4::Cmd::BindUniformBuffer(1, m_objectUbo.value());

	m_materialUboData.diffuse = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	m_materialUboData.hasDiffuseTexture = model.material.diffuseTexture != nullptr;
	m_materialUboData.hasSpecularTexture = model.material.specularTexture != nullptr;
	m_materialUboData.hasEmissionTexture = model.material.emissionTexture != nullptr;
	m_materialUboData.hasNormalMapTexture = model.material.normalTexture != nullptr;
	m_materialUboData.hasDepthMapTexture = model.material.depthTexture != nullptr;
	m_materialUboData.noLighing = model.material.noLighing;
	m_materialUbo->UpdateData(m_materialUboData);
	gl4::Cmd::BindUniformBuffer(2, m_materialUbo.value());
		
	if (model.material.diffuseTexture)
		gl4::Cmd::BindSampledImage(0, *model.material.diffuseTexture, sampler);
	if (model.material.specularTexture)
		gl4::Cmd::BindSampledImage(1, *model.material.specularTexture, sampler);
	if (model.material.emissionTexture)
		gl4::Cmd::BindSampledImage(2, *model.material.emissionTexture, sampler);
	if (model.material.normalTexture)
		gl4::Cmd::BindSampledImage(3, *model.material.normalTexture, sampler);
	if (model.material.depthTexture)
		gl4::Cmd::BindSampledImage(4, *model.material.depthTexture, sampler);

	model.mesh->Bind();
}
//=============================================================================
void GameApp3::drawModel(std::optional<GameModel> model)
{
	const gl4::Sampler& sampler = (model.value().textureFilter == gl4::MagFilter::Linear)
		? m_linearSampler.value()
		: m_nearestSampler.value();

	m_objectUboData.model = model.value().GetModelMat();
	m_objectUbo->UpdateData(m_objectUboData);
	gl4::Cmd::BindUniformBuffer(1, m_objectUbo.value());

	for (size_t i = 0; i < model.value().meshes.size(); i++)
	{
		auto& meshes = model.value().meshes[i];


		m_materialUboData.diffuse = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		m_materialUboData.hasDiffuseTexture = meshes->GetMaterial()->diffuseTexture != nullptr;
		m_materialUboData.hasSpecularTexture = meshes->GetMaterial()->specularTexture != nullptr;
		m_materialUboData.hasEmissionTexture = meshes->GetMaterial()->emissionTexture != nullptr;
		m_materialUboData.hasNormalMapTexture = meshes->GetMaterial()->normalTexture != nullptr;
		m_materialUboData.hasDepthMapTexture = meshes->GetMaterial()->depthTexture != nullptr;
		m_materialUboData.noLighing = meshes->GetMaterial()->noLighing;
		m_materialUbo->UpdateData(m_materialUboData);
		gl4::Cmd::BindUniformBuffer(2, m_materialUbo.value());

		if (meshes->GetMaterial()->diffuseTexture)
			gl4::Cmd::BindSampledImage(0, *meshes->GetMaterial()->diffuseTexture, sampler);
		if (meshes->GetMaterial()->specularTexture)
			gl4::Cmd::BindSampledImage(1, *meshes->GetMaterial()->specularTexture, sampler);
		if (meshes->GetMaterial()->emissionTexture)
			gl4::Cmd::BindSampledImage(2, *meshes->GetMaterial()->emissionTexture, sampler);
		if (meshes->GetMaterial()->normalTexture)
			gl4::Cmd::BindSampledImage(3, *meshes->GetMaterial()->normalTexture, sampler);
		if (meshes->GetMaterial()->depthTexture)
			gl4::Cmd::BindSampledImage(4, *meshes->GetMaterial()->depthTexture, sampler);

		meshes->Bind();
	}
}
//=============================================================================
bool GameApp3::createPipeline()
{
	auto vertexShader = gl4::Shader(gl4::PipelineStage::VertexShader, io::ReadShaderCode("GameData/shaders/MainShader3.vert"), "MainShader VS");
	if (!vertexShader.IsValid()) return false;
	auto fragmentShader = gl4::Shader(gl4::PipelineStage::FragmentShader, io::ReadShaderCode("GameData/shaders/MainShader3.frag"), "MainShader FS");
	if (!fragmentShader.IsValid()) return false;

	m_pipeline = gl4::GraphicsPipeline({
		.name = "Model Pipeline",
		.vertexShader = &vertexShader,
		.fragmentShader = &fragmentShader,
		.inputAssemblyState = {.topology = gl4::PrimitiveTopology::TRIANGLE_LIST},
		.vertexInputState = {MeshVertexInputBindingDescs},
		.rasterizationState = {/*.frontFace = gl4::FrontFace::Clockwise*/ .cullMode = gl4::CullMode::None },
		.depthState = {.depthTestEnable = true, .depthWriteEnable = true},
		});
	if (!m_pipeline.has_value()) return false;

	return true;
}
//=============================================================================