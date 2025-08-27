#include "stdafx.h"
#include "Input.h"
//=============================================================================
void Input::Init(GLFWwindow* wnd)
{
	Input::m_window = wnd;

	double xpos, ypos;
	glfwGetCursorPos(wnd, &xpos, &ypos);
	m_cursorOffset.x = static_cast<float>(xpos);
	m_cursorOffset.y = static_cast<float>(ypos);
	m_cursorPosLastFrame = m_cursorOffset;
}
//=============================================================================
void Input::Update()
{
	// TODO: wtf
	for (unsigned i = 0; i < MaxKeys; ++i)
	{
		// keystates decay to either up or down after one frame
		if (m_keysStatus[i] & KeyState::up) m_keysStatus[i] = KeyState::up;
		if (m_keysStatus[i] & KeyState::down) m_keysStatus[i] = KeyState::down;
	}
	// TODO: wtf
	for (unsigned i = 0; i < MaxMouseButtons; i++)
	{
		if (m_mouseButtonStatus[i] & KeyState::up) m_mouseButtonStatus[i] = KeyState::up;
		if (m_mouseButtonStatus[i] & KeyState::down) m_mouseButtonStatus[i] = KeyState::down;
	}
	m_scrollOffset = glm::vec2(0);
	m_cursorOffset = glm::vec2(0);
	glfwPollEvents();
}
//=============================================================================
void Input::keyPress(int key, int action)
{
	if (key != GLFW_KEY_UNKNOWN)
	{
		switch (action)
		{
		case GLFW_RELEASE:
			m_keysStatus[key] = KeyState::released;
			break;
		case GLFW_PRESS:
			m_keysStatus[key] = KeyState::pressed;
			break;
		case GLFW_REPEAT:
			m_keysStatus[key] = KeyState::repeat;
			break;
		default:
			std::unreachable();
			break;
		}
	}
}
//=============================================================================
void Input::mouseButton(int button, int action)
{
	switch (action)
	{
	case GLFW_RELEASE:
		m_mouseButtonStatus[button] = KeyState::released;
		break;
	case GLFW_PRESS:
		m_mouseButtonStatus[button] = KeyState::pressed;
		break;
	case GLFW_REPEAT:
		m_mouseButtonStatus[button] = KeyState::repeat;
		break;
	default:
		std::unreachable();
		break;
	}
}
//=============================================================================
void Input::mousePos(double xPos, double yPos)
{
	m_cursorPos.x = static_cast<float>(xPos);
	m_cursorPos.y = static_cast<float>(yPos);

	m_cursorOffset.x += m_cursorPos.x - m_cursorPosLastFrame.x;
	m_cursorOffset.y += m_cursorPosLastFrame.y - m_cursorPos.y;
	m_cursorPosLastFrame = m_cursorPos;
	m_cursorOffset *= sensitivity;
}
//=============================================================================
void Input::mouseScroll(double xOffset, double yOffset)
{
	m_scrollOffset.x = static_cast<float>(xOffset);
	m_scrollOffset.y = static_cast<float>(yOffset);
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