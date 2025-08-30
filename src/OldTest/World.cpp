#include "stdafx.h"
#include "World.h"
//=============================================================================
void createXZPlane(float width, float depth, int xDivisions, int zDivisions, float uvRepeat, std::vector<MeshVertex>& vertices, std::vector<uint32_t>& indices)
{
	float halfWidth = width * 0.5f,
		halfDepth = depth * 0.5f;
	float xStep = width / (float)xDivisions,
		zStep = depth / (float)zDivisions;
	float z = -halfDepth;
	for (int zI = 0; zI <= zDivisions; zI++, z += zStep)
	{
		float x = -halfWidth;
		for (int xI = 0; xI <= xDivisions; xI++, x += xStep)
		{
			MeshVertex v;
			v.position = { x, 0, z };
			v.normal = { 0, 1, 0 };
			v.texCoord = { x / width * uvRepeat, z / depth * uvRepeat };
			v.color = glm::vec3(1.0f);
			vertices.emplace_back(v);
		}
	}
	int zW = (xDivisions + 1);
	for (int zI = 0; zI < zDivisions; zI++)
	{
		int zI0 = zI * zW,
			zI1 = zI0 + zW;
		for (int xI = 0; xI < xDivisions; xI++)
		{
			int xzI0 = zI0 + xI,
				xzI1 = zI1 + xI;
			indices.emplace_back(xzI0 + 1);
			indices.emplace_back(xzI0);
			indices.emplace_back(xzI1);
			indices.emplace_back(xzI0 + 1);
			indices.emplace_back(xzI1);
			indices.emplace_back(xzI1 + 1);
		}
	}
}
//=============================================================================
bool World::Init()
{
#if 0
	std::vector<MeshVertex> vertices = {
		// positions                                // normals            // texcoords
		{{-50.0f, -0.25f,  50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  { 0.0f,  0.0f}},
		{{-50.0f, -0.25f, -50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  { 0.0f, 50.0f}},
		{{ 50.0f, -0.25f,  50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  {50.0f,  0.0f}},
		{{ 50.0f, -0.25f,  50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  {50.0f,  0.0f}},
		{{-50.0f, -0.25f, -50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  { 0.0f, 50.0f}},
		{{ 50.0f, -0.25f, -50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  {50.0f, 50.0f}}
	};
	std::vector<uint32_t> indices = { 0, 1, 2, 3, 4, 5 };
#else
	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
	createXZPlane(20, 20, 1, 1, 4, vertices, indices);
#endif

	m_model1.mesh = LoadDataMesh(vertices, indices);
	m_model1.textureFilter = gl::MagFilter::Nearest;
	m_model1.material.diffuseTexture = TextureManager::GetTexture("CoreData/textures/White1x1.png");
	m_model1.material.normalTexture = TextureManager::GetTexture("CoreData/textures/normal01.tga");

	m_model2.mesh = LoadAssimpMesh("ExampleData/mesh/bunny.obj");
	m_model2.scale = glm::vec3(3.0f);
	//m_model2.position = glm::vec3(0.0f, 0.0f, -4.0f);
	m_model2.textureFilter = gl::MagFilter::Linear;
	m_model2.material.normalTexture = TextureManager::GetTexture("CoreData/textures/normal01.tga");

	m_model3.mesh = LoadAssimpMesh("ExampleData/mesh/stall/stall.obj");
	m_model3.scale = glm::vec3(0.3f);
	m_model3.position = glm::vec3(4.0f, -0.6f, 0.0f);
	m_model3.textureFilter = gl::MagFilter::Linear;
	m_model3.material.diffuseTexture = TextureManager::GetTexture("ExampleData/mesh/stall/stallTexture.png", false);
	m_model3.material.normalTexture = TextureManager::GetTexture("CoreData/textures/normal01.tga");

	m_model4.mesh = LoadAssimpMesh("ExampleData/mesh/cube.obj");
	m_model4.position = glm::vec3(-2.0f, 0.0f, 2.0f);
	m_model4.textureFilter = gl::MagFilter::Linear;
	m_model4.material.diffuseTexture = TextureManager::GetTexture("ExampleData/textures/wood.jpg");
	m_model4.material.normalTexture = TextureManager::GetTexture("CoreData/textures/normal01.tga");

	Light l;
	l.diffuseColor = glm::vec3(1, 0, 0);
	l.diffusePower = 8.0f;
	l.specularColor = glm::vec3(1, 0, 0);
	l.specularPower = 8.0f;
	l.position = glm::vec3(4.0f, 3.0f, 4.0f);
	l.type = DIRECTIONAL;
	m_lights.push_back(l);

	l.diffuseColor = glm::vec3(0, 1, 0);
	l.diffusePower = 8.0f;
	l.specularColor = glm::vec3(0, 1, 0);
	l.specularPower = 8.0f;
	l.position = glm::vec3(-4.0f, 3.0f, 4.0f);
	l.type = DIRECTIONAL;
	m_lights.push_back(l);

	l.diffuseColor = glm::vec3(0, 0, 1);
	l.diffusePower = 8.0f;
	l.specularColor = glm::vec3(0, 0, 1);
	l.specularPower = 8.0f;
	l.position = glm::vec3(4.0f, 3.0f, -4.0f);
	l.type = DIRECTIONAL;
	m_lights.push_back(l);

	l.diffuseColor = glm::vec3(1, 1, 0);
	l.diffusePower = 8.0f;
	l.specularColor = glm::vec3(1, 1, 0);
	l.specularPower = 8.0f;
	l.position = glm::vec3(-4.0f, 3.0f, -4.0f);
	l.type = DIRECTIONAL;
	m_lights.push_back(l);

	l.diffuseColor = glm::vec3(0, 1, 1);
	l.diffusePower = 8.0f;
	l.specularColor = glm::vec3(0, 1, 1);
	l.specularPower = 8.0f;
	l.position = glm::vec3(0.0f, 3.0f, 0.0f);
	l.type = DIRECTIONAL;
	m_lights.push_back(l);

	m_shadows.resize(1);

	m_shadows[0].hasCubeMap = false;
	m_shadows[0].shadowLightPos = { 2.0f, 2.0f, 1.0f };// l.position;
	m_shadows[0].Create();

	return true;
}
//=============================================================================
void World::Close()
{
	for (size_t i = 0; i < m_shadows.size(); i++)
	{
		m_shadows[i].Destroy();
	}
	m_shadows.clear();

	delete m_model1.mesh;
	delete m_model2.mesh;
	delete m_model3.mesh;
	delete m_model4.mesh;
}
//=============================================================================