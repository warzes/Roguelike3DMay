#include "stdafx.h"
#include "GameObject.h"
//=============================================================================
void GameObject::SetPosition(const glm::vec3& position)
{
	m_position = position;
}
//=============================================================================
void GameObject::SetScale(const glm::vec3& scale)
{
	m_scale = scale;
}
//=============================================================================
void GameObject::SetRotation(const glm::vec3& rotation)
{
	m_rotation = rotation;
}
//=============================================================================
void GameObject::UpdateModelMatrix()
{
	// 1. Масштаб
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_scale);

	// 2. Вращение (углы Эйлера в радианах)
	 glm::mat4 rotate = glm::eulerAngleXYZ(m_rotation.x, m_rotation.y, m_rotation.z);

	// 3. Перенос
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), m_position);

	// 4. Объединение: T * R * S
	m_modelMatrix = translate * rotate * scale;
}
//=============================================================================
void GameObject::SetName(const std::string& name)
{
	m_name = name;
}
//=============================================================================
void GameObject::SetChildCount(size_t count)
{
	m_childCount = count;
}
//=============================================================================
void GameObject::AddChild(GameObject* child)
{
	child->m_parent = this;
	if (m_childCount == 0)
		m_firstChild = child;
	else
	{
		GameObject* last = GetLastChild();
		last->m_next = child;
		child->m_previous = last;
	}

	m_childCount++;
}
//=============================================================================
void GameObject::SetFirstChild(GameObject* child)
{
	m_firstChild = child;
}
//=============================================================================
void GameObject::SetParent(GameObject* parent)
{
	m_parent = parent;
	m_parent->AddChild(this);
}
//=============================================================================
void GameObject::SetPrevious(GameObject* previous)
{
	m_previous = previous;
}
//=============================================================================
void GameObject::SetNext(GameObject* next)
{
	m_next = next;
}
//=============================================================================
void GameObject::SetVisible(bool v)
{
	m_visible = v;
}
//=============================================================================
void GameObject::SetVisible(bool v, bool setAllChild, bool setAllParent)
{
	m_visible = v;

	if (setAllParent)
	{
		GameObject* curParent = m_parent;
		while (curParent != nullptr)
		{
			GameObject* curChild = curParent->GetFirstChild();
			bool isParentVisible = v;
			while (curChild != nullptr)
			{
				isParentVisible = isParentVisible || curChild->IsVisible();
				curChild = curChild->GetNext();
			}
			curParent->SetVisible(isParentVisible, false, true);
			curParent = curParent->GetParent();
		}
	}

	if (setAllChild)
	{
		GameObject* curChild = m_firstChild;
		while (curChild != nullptr)
		{
			curChild->SetVisible(v, true, false);
			curChild = curChild->GetNext();
		}
	}
}
//=============================================================================
const glm::vec3& GameObject::GetPosition() const
{
	return m_position;
}
//=============================================================================
const glm::vec3& GameObject::GetScale() const
{
	return m_scale;
}
//=============================================================================
const glm::vec3& GameObject::GetRotation() const
{
	return m_rotation;
}
//=============================================================================
const glm::vec3& GameObject::GetForward() const
{
	// TODO: проверить.
	return glm::normalize(glm::vec3(m_modelMatrix[2]));
}
//=============================================================================
const glm::vec3& GameObject::GetRight() const
{
	// TODO: проверить.
	return glm::normalize(glm::vec3(m_modelMatrix[0]));
}
//=============================================================================
const glm::vec3& GameObject::GetUp() const
{
	// TODO: проверить.
	return glm::normalize(glm::vec3(m_modelMatrix[1]));
}
//=============================================================================
const glm::mat4& GameObject::GetModelMatrix() const
{
	return m_modelMatrix;
}
//=============================================================================
std::string GameObject::GetName() const
{
	return m_name;
}
//=============================================================================
size_t GameObject::GetChildCount() const
{
	return m_childCount;
}
//=============================================================================
GameObject* GameObject::GetParent()
{
	return m_parent;
}
//=============================================================================
GameObject* GameObject::GetFirstChild()
{
	return m_firstChild;
}
//=============================================================================
GameObject* GameObject::GetPrevious()
{
	return m_previous;
}
//=============================================================================
GameObject* GameObject::GetNext()
{
	return m_next;
}
//=============================================================================
GameObject* GameObject::GetLastChild()
{
	if (m_childCount == 0)
		return nullptr;

	GameObject* current = m_firstChild;
	while (true)
	{
		if (current->m_next == nullptr)
			return current;
		else
			current = current->m_next;
	}
}
//=============================================================================
GameObjectType GameObject::GetType()
{
	return m_type;
}
//=============================================================================
bool GameObject::IsVisible() const
{
	return m_visible;
}
//=============================================================================
void GameObject::LookAt(const glm::vec3& p)
{
	glm::vec3 v = p - m_position;
	glm::vec3 vXZ = glm::normalize(glm::vec3(v.x, 0.0f, v.z));
	v = glm::normalize(v);

	float thetaY = glm::dot(glm::vec3(0.0f, 0.0f, -1.0f), vXZ);
	thetaY = acos(thetaY) * 180.0f / glm::pi<float>();
	if (v.x < 0.0f) thetaY = -thetaY;

	float thetaX = glm::dot(v, vXZ);
	thetaX = acos(thetaX) * 180.0f / glm::pi<float>();
	if (v.y > 0.0f) thetaX = -thetaX;

	SetRotation(glm::vec3(thetaX, thetaY, m_rotation.z));
}
//=============================================================================