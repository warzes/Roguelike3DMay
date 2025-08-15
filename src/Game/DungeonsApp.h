#pragma once

#include "GameModel.h"
#include "UniformObjects.h"
#include "DungeonsUniforms.h"

namespace dung
{
	/*Simple point light struct which can be stored as a matrix4*/
	struct PointLight
	{
		PointLight(glm::vec3 pos, glm::vec3 color, float fallOffStart, float fallOffEnd, float luminance) : pos(pos), color(color), fallOffStart(fallOffStart), fallOffEnd(fallOffEnd), luminance(luminance) {};
		glm::vec3 pos;
		glm::vec3 color;
		float fallOffStart = 1.0F;
		float fallOffEnd = 10.0F;
		float luminance = 1.0F;

		static constexpr int sizeInBytes = 9 * sizeof(float);

		glm::mat4 getMatrix() { return glm::mat4(pos.x, pos.y, pos.z, 0, color.x, color.y, color.z, 0, fallOffStart, fallOffEnd, luminance, 0, 0, 0, 0, 0); }
	};
	/*

Point light matrix layout:

pos.x			color.r			fallOffStart		0
pos.y			color.g			fallOffEnd			0
pos.z			color.b			luminance			0
0				0				0					0

*/

	class DungeonsApp final : public IEngineApp
	{
	public:
		DungeonsApp();
		DungeonsApp(const DungeonsApp&) = delete;
		DungeonsApp(DungeonsApp&&) = delete;
		void operator=(const DungeonsApp&) = delete;
		void operator=(DungeonsApp&&) = delete;

		EngineCreateInfo GetCreateInfo() const final;

		bool OnInit() final;
		void OnClose() final;
		void OnUpdate(float deltaTime) final;
		void OnRender() final;
		void OnImGuiDraw() final;
		void OnResize(uint16_t width, uint16_t height) final;
		void OnMouseButton(int button, int action, int mods) final;
		void OnMousePos(double x, double y) final;
		void OnScroll(double dx, double dy) final;
		void OnKey(int key, int scanCode, int action, int mods) final;

		Camera& GetCamera() { return m_camera; }

	private:
		bool createPipeline();
		void drawModel(GameModelOld& model);
		void drawModel(std::optional<GameModel> model);

		std::optional<gl::Texture> m_finalColorBuffer;
		std::optional<gl::Texture> m_finalDepthBuffer;

		Camera                     m_camera;
		glm::mat4                  m_projection;

		GameModelOld m_model1;
		GameModelOld m_model2;
		std::optional<GameModel> m_model3;

		std::optional<gl::GraphicsPipeline>              m_pipeline;
		std::optional<gl::TypedBuffer<SceneUniforms>>    m_sceneUbo;
		SceneUniforms                                    m_sceneUboData;
		std::optional<gl::TypedBuffer<ModelObjUniforms>> m_objectUbo;
		ModelObjUniforms                                 m_objectUboData;
		std::optional<gl::TypedBuffer<LightUniforms>>    m_lightUbo;
		LightUniforms                                    m_lightUboData;
		std::optional<gl::TypedBuffer<MaterialUniforms>> m_materialUbo;
		MaterialUniforms                                 m_materialUboData;

		std::optional<gl::Sampler>                       m_nearestSampler;
		std::optional<gl::Sampler>                       m_linearSampler;

		std::vector<PointLight> m_pointLights;
	};
} // namespace dung