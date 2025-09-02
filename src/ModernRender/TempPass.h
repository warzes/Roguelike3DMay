#pragma once

class TempPass final
{
public:
	bool Init();
	void Close();

	void Begin();
	void End();

private:
	RenderTarget m_rt;
};