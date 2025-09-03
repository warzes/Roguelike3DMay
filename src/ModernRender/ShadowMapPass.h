#pragma once

class ShadowMapPass final
{
	friend class RenderPassManager;
public:
	bool Init();
	void Close();

	void Begin();
	void End();

private:
	RenderTarget                        m_rt;
	std::optional<gl::GraphicsPipeline> m_pipeline;
};