#include "stdafx.h"
#include "TempApp.h"

namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// Interface block
out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

layout(binding = 0) uniform Matrices { 
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 proj;
};

void main()
{
	vs_out.FragPos = aPos;
	vs_out.Normal = aNormal; // TODO transpose(inverse(mat3(model))) * aNormal;
	vs_out.TexCoords = aTexCoords;
	gl_Position = proj * view * model * vec4(aPos, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

layout(binding = 0) uniform sampler2D diffuseTex;

vec3 lightPos = vec3(4.0, 5.0, -3.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);

layout(location = 0) out vec4 FragColor;

void main()
{
	float lightAngle = max(dot(normalize(fs_in.Normal), normalize(lightPos)), 0.0);
	vec4 color = texture(diffuseTex, fs_in.TexCoords);

	FragColor = vec4(color.rgb, color.a) * vec4((0.3 + 0.7 * lightAngle) * lightColor, 1.0);
}
)";

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	struct UBO final
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	constexpr std::array<gl4::VertexInputBindingDescription, 3> inputBindingDescs{
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
		.offset = offsetof(Vertex, normal),
	  },
		gl4::VertexInputBindingDescription{
		.location = 2,
		.binding = 0,
		.format = gl4::Format::R32G32_FLOAT,
		.offset = offsetof(Vertex, uv),
	  },
	};

	Mesh* mesh;
	//std::optional<gl4::Buffer> vertexBuffer1;
	//std::optional<gl4::Buffer> indexBuffer;
	gl4::Texture* diffuse;
	std::optional<gl4::Sampler> sampler;

	std::optional<gl4::TypedBuffer<UBO>> uniformBuffer1;
	std::optional<gl4::GraphicsPipeline> pipeline;
	std::optional<gl4::Texture> msColorTex;
	std::optional<gl4::Texture> gDepth;

	// tes mesh
	std::vector<Vertex> mv;
	std::vector<uint32_t> miv;
	std::optional<gl4::Buffer> mVB;
	std::optional<gl4::Buffer> mIB;

	gl4::GraphicsPipeline CreatePipeline()
	{
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

	Camera camera;
}
//=============================================================================
EngineCreateInfo TempApp::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool TempApp::OnInit()
{
	std::vector<MeshVertex> vertices = {
		// positions            // normals            // texcoords
		{{-10.0f, -0.5f,  10.0f},  {0.0f, 1.0f, 0.0f},  { 0.0f,  0.0f}},
		{{-10.0f, -0.5f, -10.0f},  {0.0f, 1.0f, 0.0f},  { 0.0f, 10.0f}},
		{{ 10.0f, -0.5f,  10.0f},  {0.0f, 1.0f, 0.0f},  {10.0f,  0.0f}},
		{{ 10.0f, -0.5f,  10.0f},  {0.0f, 1.0f, 0.0f},  {10.0f,  0.0f}},
		{{-10.0f, -0.5f, -10.0f},  {0.0f, 1.0f, 0.0f},  { 0.0f, 10.0f}},
		{{ 10.0f, -0.5f, -10.0f},  {0.0f, 1.0f, 0.0f},  {10.0f, 10.0f}}
	};
	std::vector<uint32_t> iv = { 0, 1, 2, 3, 5 };

	mesh = new Mesh(vertices, iv, std::nullopt);


	{
		const aiScene* scene = aiImportFile("CoreData/mesh/Cube/Cube.gltf", ASSIMP_LOAD_FLAGS);
		if (!scene || !scene->HasMeshes())
		{
			Fatal("Not load mesh");
			return false;
		}

		const aiMesh* mesh = scene->mMeshes[0];
		mv.resize(mesh->mNumVertices);
		for (size_t i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex& nv = mv[i];

			nv.pos.x = mesh->mVertices[i].x;
			nv.pos.y = mesh->mVertices[i].y;
			nv.pos.z = mesh->mVertices[i].z;

			//if (mesh->HasNormals())
			{
				nv.normal.x = mesh->mNormals[i].x;
				nv.normal.y = mesh->mNormals[i].y;
				nv.normal.z = mesh->mNormals[i].z;
			}

			//if (mesh->HasTextureCoords(0))
			{
				nv.uv.x = mesh->mTextureCoords[0][i].x;
				nv.uv.y = mesh->mTextureCoords[0][i].y;
				const auto tc = mesh->mTextureCoords[0][i];
			}
		}
		//miv.resize(mesh->mNumFaces * 3);
		for (size_t i = 0; i < mesh->mNumFaces; i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				miv.emplace_back(mesh->mFaces[i].mIndices[j]);
			}
		}

		aiReleaseImport(scene);

		mVB = gl4::Buffer(std::span(mv));
		mIB = gl4::Buffer(std::span(miv));
	}

	uniformBuffer1 = gl4::TypedBuffer<UBO>(gl4::BufferStorageFlag::DynamicStorage);

	// Load Texture
	diffuse = TextureManager::GetTexture("CoreData/textures/colorful.png");

	gl4::SamplerState ss;
	ss.minFilter = gl4::MinFilter::Nearest;
	ss.magFilter = gl4::MagFilter::Nearest;
	ss.addressModeU = gl4::AddressMode::Repeat;
	ss.addressModeV = gl4::AddressMode::Repeat;
	sampler = gl4::Sampler(ss);

	pipeline = CreatePipeline();

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	resize(GetWindowWidth(), GetWindowHeight());

	glEnable(GL_MULTISAMPLE); // for shadows

	return true;
}
//=============================================================================
void TempApp::OnClose()
{
	delete mesh;
	uniformBuffer1 = {};
	uniformBuffer1 = {};
	pipeline = {};
	msColorTex = {};
}
//=============================================================================
void TempApp::OnUpdate(float deltaTime)
{
	if (Input::IsKeyDown(GLFW_KEY_W)) camera.ProcessKeyboard(CameraForward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_S)) camera.ProcessKeyboard(CameraBackward, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_A)) camera.ProcessKeyboard(CameraLeft, deltaTime);
	if (Input::IsKeyDown(GLFW_KEY_D)) camera.ProcessKeyboard(CameraRight, deltaTime);

	if (Input::IsMouseDown(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(false);
		camera.ProcessMouseMovement(-Input::GetScreenOffset().x, Input::GetScreenOffset().y);
	}
	else if (Input::IsMouseReleased(GLFW_MOUSE_BUTTON_RIGHT))
	{
		Input::SetCursorVisible(true);
	}

	UBO ubo;
	ubo.model = glm::mat4(1.0f);
	ubo.view = camera.GetViewMatrix();
	ubo.proj = glm::perspective(glm::radians(45.0f), GetWindowAspect(), 0.01f, 1000.0f);

	uniformBuffer1->UpdateData(ubo);
}
//=============================================================================
void TempApp::OnRender()
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
		mesh->Bind();
		//gl4::Cmd::BindVertexBuffer(0, vertexBuffer1.value(), 0, sizeof(Vertex));
		gl4::Cmd::BindUniformBuffer(0, uniformBuffer1.value());
		gl4::Cmd::BindSampledImage(0, *diffuse, sampler.value());
		gl4::Cmd::Draw(6, 1, 0, 0);

		gl4::Cmd::BindGraphicsPipeline(pipeline.value());
		gl4::Cmd::BindVertexBuffer(0, mVB.value(), 0, sizeof(Vertex));
		gl4::Cmd::BindIndexBuffer(mIB.value(), gl4::IndexType::UNSIGNED_INT);
		gl4::Cmd::BindUniformBuffer(0, uniformBuffer1.value());
		gl4::Cmd::BindSampledImage(0, *diffuse, sampler.value());
		gl4::Cmd::DrawIndexed(miv.size(), 1, 0, 0, 0);
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
void TempApp::OnImGuiDraw()
{
	DrawFPS();
}
//=============================================================================
void TempApp::OnResize(uint16_t width, uint16_t height)
{
	resize(width, height);
}
//=============================================================================
void TempApp::OnMouseButton(int button, int action, int mods)
{
}
//=============================================================================
void TempApp::OnMousePos(double x, double y)
{
}
//=============================================================================
void TempApp::OnScroll(double dx, double dy)
{
}
//=============================================================================
void TempApp::OnKey(int key, int scanCode, int action, int mods)
{
}
//=============================================================================