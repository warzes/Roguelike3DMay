#pragma once

struct alignas(16) DepthBlock final
{
	glm::mat4 vp;
};

class DepthPass final
{
	friend class RenderPassManager;
public:
	bool Init();
	void Close();

	void Begin();
	void End();

	const gl::Texture* GetTexture() const;

	const glm::mat4& GetMatrix() const { return m_depthDataUBO.Get().vp; }

private:
	RenderTarget                        m_rt;
	std::optional<gl::GraphicsPipeline> m_pipeline;
	UniformsWrapper<DepthBlock>         m_depthDataUBO;
};