#pragma once

class GameApp;

namespace game
{
	struct ViewUBO final
	{
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct TransformUBO final
	{
		glm::mat4 model;
	};
}

class GameGraphics final
{
public:
	bool Init(GameApp* gameApp);
	void Close();
	void Update(float deltaTime);
	void Render();

	void Resize(uint16_t width, uint16_t height);

	Camera& GetCamera() { return camera; }

private:
	gl4::GraphicsPipeline CreatePipeline()
	{
		auto vertexShader = gl4::Shader(gl4::PipelineStage::VertexShader, FileUtils::ReadShaderCode("GameData/shaders/Minimal.vert"), "min VS");
		auto fragmentShader = gl4::Shader(gl4::PipelineStage::FragmentShader, FileUtils::ReadShaderCode("GameData/shaders/Minimal.frag"), "min FS");

		return gl4::GraphicsPipeline({
			 .name = "Triangle Pipeline",
			.vertexShader = &vertexShader,
			.fragmentShader = &fragmentShader,
			.inputAssemblyState = {.topology = gl4::PrimitiveTopology::TRIANGLE_LIST},
			.vertexInputState = {MeshVertexInputBindingDescs},
			.depthState = {.depthTestEnable = true},
			});
	}

	GameApp* m_gameApp{ nullptr };

	Mesh* mesh1;
	gl4::Texture* diffuse;
	gl4::Texture* diffuseStall;
	std::optional<gl4::Sampler> sampler;

	std::optional<gl4::TypedBuffer<game::ViewUBO>> ubo1;
	std::optional<gl4::TypedBuffer<game::TransformUBO>> ubo2;

	std::optional<gl4::GraphicsPipeline> pipeline;
	std::optional<gl4::Texture> msColorTex;
	std::optional<gl4::Texture> gDepth;

	Mesh* mesh2;
	Mesh* mesh3;

	Camera camera;
};