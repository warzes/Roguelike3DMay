#pragma once

class Temp2Pass final
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