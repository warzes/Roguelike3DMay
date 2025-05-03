#pragma once

void ExitApp();

struct EngineConfig final
{
	struct Window final
	{
		uint16_t         width{ 1600 };
		uint16_t         height{ 900 };
		std::string_view title{ "Game" };
	} window;
};

class IEngineApp
{
public:
	IEngineApp() = default;
	virtual ~IEngineApp() = default;

	IEngineApp(const IEngineApp&) = delete;
	IEngineApp(IEngineApp&&) = delete;
	void operator=(const IEngineApp&) = delete;
	void operator=(IEngineApp&&) = delete;

	void Run();

	virtual EngineConfig GetConfig() const = 0;
	virtual bool OnCreate() = 0;
	virtual void OnDestroy() = 0;

	uint16_t GetWidth() const { return m_width; }
	uint16_t GetHeight() const { return m_height; }

private:
	bool create();
	bool shouldWindowClose() const;
	void destroy();

	GLFWwindow* m_window;
	uint16_t m_width;
	uint16_t m_height;
};