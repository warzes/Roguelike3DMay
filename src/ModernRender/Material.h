#pragma once

class Material final
{
public:
	friend class MaterialManager;

	void SetName(const std::string& n);
	void SetRoughness(float r);
	void SetMetallic(float m);
	void SetDiffuse(const std::string& dir);
	void SetNormal(const std::string& dir);
	void SetSpecular(const std::string& dir);
	void SetColor(const glm::vec4& c);
	void SetDiffuseTilling(float x, float y);
	void SetNormalTilling(float x, float y);
	void SetSpecularTilling(float x, float y);
	void SetEmissionColor(const glm::vec3& c);
	void SetEmissionIntensity(float i);
	void SetAlphaTestThreshold(float t);

	void RemoveDiffuse();
	void RemoveNormal();
	void RemoveSpecular();

	unsigned int GetID() const;
	std::string GetName() const;
	float GetMetallic() const;
	float GetRoughness() const;
	std::string GetDiffusePath() const;
	gl::Texture* GetDiffuse() const;
	std::string GetNormalPath() const;
	gl::Texture* GetNormal() const;
	std::string GetSpecularPath() const;
	gl::Texture* GetSpecular() const;
	glm::vec4 GetColor() const;
	glm::vec3 GetDiffuseTilling() const;
	glm::vec3 GetNormalTilling() const;
	glm::vec3 GetSpecularTilling() const;
	glm::vec3 GetEmissionColor() const;
	float GetEmissionIntensity() const;
	float GetAlphaTestThreshold() const;
	bool HasDiffuseTexture() const;
	bool HasNormalTexture() const;
	bool HasSpecularTexture() const;

private:
	Material();
	~Material();

	void setTexture(const std::string& dir, int index);
	void setID(unsigned int newID);

	unsigned int m_id;
	std::string  m_name;
	std::string  m_texDirs[3];
	gl::Texture* m_texIns[3];
	bool         m_hasTex[3];
	float        m_texTilling[6];
	float        m_roughness;
	float        m_metallic;
	glm::vec4    m_color;
	glm::vec3    m_emissionColor;
	float        m_emissionIntensity;
	float        m_alphaTestThreshold;

};