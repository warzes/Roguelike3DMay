#pragma once

class DirectionalLight final
{
public:
	void SetPosition(const glm::vec3& position, const glm::vec3& targetView = glm::vec3(0.0f, 0.0f, 0.0f));
	void SetColor(const glm::vec3& color, float intensity);
	
	const glm::vec3& GetPosition() const;
	const glm::vec3& GetTargetView() const;
	const glm::vec3& GetDirectional() const;
	const glm::mat4& GetMatrix();

	const glm::vec3& GetColor() const;
	float GetIntensity() const;

	bool IsNeedsUpdate() const { return m_needsUpdate; }

private:
	glm::vec3 m_position = glm::vec3(-20.0f, 10.0f, -3.0f);
	glm::vec3 m_targetView = glm::vec3(0.0f);

	glm::vec3 m_color = glm::vec3(1.0f, 0.6f, 0.2f);
	float     m_intensity = 3.0f;

	glm::mat4 m_viewProj{ glm::mat4(1.0f) };

	bool      m_needsUpdate{ true };
};

inline DirectionalLight gDirectionalLight;