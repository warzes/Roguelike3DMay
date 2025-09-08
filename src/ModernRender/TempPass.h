#pragma once

class ForwardPass final
{
	friend class RenderPassManager;
public:
	bool Init();
	void Close();

	void Begin(const glm::vec3& clearColor);
	void End();

private:
	RenderTarget                        m_rt;
	std::optional<gl::GraphicsPipeline> m_pipeline;
};