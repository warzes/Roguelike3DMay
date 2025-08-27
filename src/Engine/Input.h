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

	static const glm::vec2& GetCursorPos() { return m_cursorPos; }
	static const glm::vec2& GetCursorOffset() { return m_cursorOffset; }
	static const glm::vec2& GetPrevCursorPos() { return m_cursorPosLastFrame; }
	static const glm::vec2& GetScrollOffset() { return m_scrollOffset; }

	static KeyState GetKeyState(Key key) { return m_keysStatus[key]; }
	static bool IsKeyDown(Key key) { return m_keysStatus[key] & KeyState::down; }
	static bool IsKeyUp(Key key) { return m_keysStatus[key] & KeyState::up; }
	static bool IsKeyPressed(Key key) { return m_keysStatus[key] == KeyState::pressed; }
	static bool IsKeyReleased(Key key) { return m_keysStatus[key] == KeyState::released; }
	static bool IsMouseDown(MouseButton key) { return m_mouseButtonStatus[key] & KeyState::down; }
	static bool IsMouseUp(MouseButton key) { return m_mouseButtonStatus[key] & KeyState::up; }
	static bool IsMousePressed(MouseButton key) { return m_mouseButtonStatus[key] == KeyState::pressed; }
	static bool IsMouseReleased(MouseButton key) { return m_mouseButtonStatus[key] == KeyState::released; }

	static void SetCursorVisible(bool state);

	static inline float sensitivity = 0.35f;

private:
	friend class IEngineApp;

	static void keyPress(int key, int action);
	static void mouseButton(int button, int action);
	static void mousePos(double xPos, double yPos);
	static void mouseScroll(double xOffset, double yOffset);

	static inline glm::vec2 m_cursorPos{};
	static inline glm::vec2 m_cursorOffset{};
	static inline glm::vec2 m_cursorPosLastFrame{};
	static inline glm::vec2 m_scrollOffset{};

	static inline GLFWwindow* m_window{ nullptr };
	static inline KeyState m_keysStatus[MaxKeys] = { KeyState(0) };
	static inline KeyState m_mouseButtonStatus[MaxMouseButtons] = { KeyState(0) };
};