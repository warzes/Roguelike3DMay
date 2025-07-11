#include "stdafx.h"
#include "Input.h"
//=============================================================================
void Input::Init(GLFWwindow* wnd)
{
	Input::m_window = wnd;
}
//=============================================================================
void Input::Update()
{
	for (unsigned i = 0; i < MaxKeys; ++i)
	{
		// keystates decay to either up or down after one frame
		if (m_keyStates[i] & KeyState::up) m_keyStates[i] = KeyState::up;
		if (m_keyStates[i] & KeyState::down) m_keyStates[i] = KeyState::down;
	}
	for (unsigned i = 0; i < MaxMouseButtons; i++)
	{
		if (m_mouseButtonStates[i] & KeyState::up) m_mouseButtonStates[i] = KeyState::up;
		if (m_mouseButtonStates[i] & KeyState::down) m_mouseButtonStates[i] = KeyState::down;
	}
	m_scrollOffset = glm::vec2(0);
	m_screenOffset = glm::vec2(0);
	glfwPollEvents();
}
//=============================================================================
Input::KeyState Input::GetKeyState(Key key)
{
	return m_keyStates[key];
}
//=============================================================================
bool Input::IsKeyDown(Key key)
{
	return m_keyStates[key] & KeyState::down;
}
//=============================================================================
bool Input::IsKeyUp(Key key)
{
	return m_keyStates[key] & KeyState::up;
}
//=============================================================================
bool Input::IsKeyPressed(Key key)
{
	return m_keyStates[key] == KeyState::pressed;
}
//=============================================================================
bool Input::IsKeyReleased(Key key)
{
	return m_keyStates[key] == KeyState::released;
}
//=============================================================================
bool Input::IsMouseDown(MouseButton key)
{
	return m_mouseButtonStates[key] & KeyState::down;
}
//=============================================================================
bool Input::IsMouseUp(MouseButton key)
{
	return m_mouseButtonStates[key] & KeyState::up;
}
//=============================================================================
bool Input::IsMousePressed(MouseButton key)
{
	return m_mouseButtonStates[key] == KeyState::pressed;
}
//=============================================================================
bool Input::IsMouseReleased(MouseButton key)
{
	return m_mouseButtonStates[key] == KeyState::released;
}
//=============================================================================
void Input::keypress(int key, int action)
{
	if (key != GLFW_KEY_UNKNOWN)
	{
		switch (action)
		{
		case GLFW_RELEASE:
			m_keyStates[key] = KeyState::released;
			break;
		case GLFW_PRESS:
			m_keyStates[key] = KeyState::pressed;
			break;
		case GLFW_REPEAT:
			m_keyStates[key] = KeyState::repeat;
			break;
		default:
			assert(0 && "Invalid keycode.");
			break;
		}
	}
}
//=============================================================================
void Input::mousePos(double xpos, double ypos)
{
	static bool firstMouse = true;

	if (firstMouse)
	{
		m_screenOffset.x = static_cast<float>(xpos);
		m_screenOffset.y = static_cast<float>(ypos);
		m_prevScreenPos = m_screenOffset;
		firstMouse = false;
		return;
	}

	m_screenPos.x = static_cast<float>(xpos);
	m_screenPos.y = static_cast<float>(ypos);

	m_screenOffset.x += static_cast<float>(xpos) - m_prevScreenPos.x;
	m_screenOffset.y += m_prevScreenPos.y - static_cast<float>(ypos);
	m_prevScreenPos = glm::vec2(xpos, ypos);
	m_screenOffset *= sensitivity;
}
//=============================================================================
void Input::mouseScroll(double xoffset, double yoffset)
{
	m_scrollOffset.x = static_cast<float>(xoffset);
	m_scrollOffset.y = static_cast<float>(yoffset);
}
//=============================================================================
void Input::mouseButton(int button, int action)
{
	switch (action)
	{
	case GLFW_RELEASE:
		m_mouseButtonStates[button] = KeyState::released;
		break;
	case GLFW_PRESS:
		m_mouseButtonStates[button] = KeyState::pressed;
		break;
	case GLFW_REPEAT:
		m_mouseButtonStates[button] = KeyState::repeat;
		break;
	default:
		assert(0 && "Invalid keycode.");
		break;
	}
}
//=============================================================================
void Input::SetCursorVisible(bool state)
{
	if (state)
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	else
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
//=============================================================================