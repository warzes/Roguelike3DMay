#pragma once

enum class GameObjectType : uint8_t
{
	Null,
	Model,
	Light,
	Camera,
};

// TODO: сейчас наследую от этого класса классы Model/Light/Camera. Вместо этого возможно надо их включать

class GameObject
{
public:
	GameObject() = default;
	virtual ~GameObject() = default;

	void SetPosition(const glm::vec3& position);
	void SetScale(const glm::vec3& scale);
	void SetRotation(const glm::vec3& rotation);
	void UpdateModelMatrix();

	void SetName(const std::string& name);
	void SetChildCount(size_t count);
	void AddChild(GameObject* child);
	void SetFirstChild(GameObject* child);
	void SetParent(GameObject* parent);
	void SetPrevious(GameObject* previous);
	void SetNext(GameObject* next);
	void SetVisible(bool v);
	void SetVisible(bool v, bool setAllChild, bool setAllParent);

	const glm::vec3& GetPosition() const;
	const glm::vec3& GetScale() const;
	const glm::vec3& GetRotation() const;
	const glm::vec3& GetForward() const;
	const glm::vec3& GetRight() const;
	const glm::vec3& GetUp() const;
	const glm::mat4& GetModelMatrix() const;
	std::string GetName() const;
	size_t GetChildCount() const;
	GameObject* GetParent();
	GameObject* GetFirstChild();
	GameObject* GetPrevious();
	GameObject* GetNext();
	GameObject* GetLastChild();
	GameObjectType GetType();
	bool IsVisible() const;
	void LookAt(const glm::vec3& p);

protected:
	GameObjectType m_type{ GameObjectType::Null };
	glm::vec3      m_position{ 0.0f };
	glm::vec3      m_scale{ 1.0f };
	glm::vec3      m_rotation{ 0.0f };
	glm::mat4      m_modelMatrix{ glm::mat4(1.0f) };
	std::string    m_name{ "Object" };
	bool           m_visible{ true };
	size_t         m_childCount{ 0 };
	GameObject*    m_parent{ nullptr };
	GameObject*    m_firstChild{ nullptr };
	GameObject*    m_previous{ nullptr };
	GameObject*    m_next{ nullptr };
};