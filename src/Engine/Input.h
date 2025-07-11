#pragma once

#include "KeyCode.h"

//enum class MouseButton : uint8_t
//{
//	Left,
//	Right,
//	Middle
//};

enum CursorState : uint8_t
{
	Normal,
	Hidden,
	Disabled
};

constexpr inline size_t MaxKeys{ GLFW_KEY_LAST };
constexpr inline size_t MaxMouseButtons{ GLFW_MOUSE_BUTTON_LAST };

// keycodes can be negative in case of an error
using Key = int;
using MouseButton = int;

struct InputKey { Key key{}; };
struct InputMouseButton { MouseButton button; };
struct InputMouseScroll { bool yaxis{ false }; };
struct InputMousePos { bool yaxis{ false }; };

class Input final
{
public:
	enum KeyState : uint8_t // applicable to keyboard keys and mouse buttons
	{
		down = 0b00001,
		pressed = 0b00011,
		up = 0b00100,
		released = 0b01100,
		repeat = 0b10001
	};
	static void Init(GLFWwindow* window);
	static void Update();

	static glm::vec2 GetScreenPos() { return m_screenPos; }
	static glm::vec2 GetScreenOffset() { return m_screenOffset; }
	static glm::vec2 GetPrevScreenPos() { return m_prevScreenPos; }
	static glm::vec2 GetScrollOffset() { return m_scrollOffset; }

	static KeyState GetKeyState(Key key);
	static bool IsKeyDown(Key key);
	static bool IsKeyUp(Key key);
	static bool IsKeyPressed(Key key);
	static bool IsKeyReleased(Key key);
	static bool IsMouseDown(MouseButton key);
	static bool IsMouseUp(MouseButton key);
	static bool IsMousePressed(MouseButton key);
	static bool IsMouseReleased(MouseButton key);

	static void SetCursorVisible(bool state);

	static inline float sensitivity = 0.35f;

private:
	friend class IEngineApp;

	static void keypress(int key, int action);
	static void mousePos(double xpos, double ypos);
	static void mouseScroll(double xoffset, double yoffset);
	static void mouseButton(int button, int action);

	static inline glm::vec2 m_screenPos{};
	static inline glm::vec2 m_screenOffset{};
	static inline glm::vec2 m_prevScreenPos{};
	static inline glm::vec2 m_scrollOffset{};

	static inline GLFWwindow* m_window{ nullptr };
	static inline KeyState m_keyStates[MaxKeys] = { KeyState(0) };
	static inline KeyState m_mouseButtonStates[MaxMouseButtons] = { KeyState(0) };
};