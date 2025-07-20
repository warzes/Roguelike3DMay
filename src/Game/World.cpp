#include "stdafx.h"
#include "World.h"
//=============================================================================
bool World::Init()
{
	std::vector<MeshVertex> vertices = {
		// positions                                // normals            // texcoords
		{{-50.0f, -0.5f,  50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  { 0.0f,  0.0f}},
		{{-50.0f, -0.5f, -50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  { 0.0f, 50.0f}},
		{{ 50.0f, -0.5f,  50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  {50.0f,  0.0f}},
		{{ 50.0f, -0.5f,  50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  {50.0f,  0.0f}},
		{{-50.0f, -0.5f, -50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  { 0.0f, 50.0f}},
		{{ 50.0f, -0.5f, -50.0f}, glm::vec3(1.0f), {0.0f, 1.0f, 0.0f},  {50.0f, 50.0f}}
	};
	std::vector<uint32_t> iv = { 0, 1, 2, 3, 4, 5 };

	m_model1.mesh = LoadDataMesh(vertices, iv);
	m_model1.textureFilter = gl4::MagFilter::Nearest;
	m_model1.material.diffuseTexture = TextureManager::GetTexture("CoreData/textures/colorful.png");
	m_model1.material.normalTexture = TextureManager::GetTexture("CoreData/textures/normal01.tga");

	m_model2.mesh = LoadAssimpMesh("ExampleData/mesh/bunny.obj");
	m_model2.scale = glm::vec3(3.0f);
	//m_model2.position = glm::vec3(0.0f, 0.0f, -4.0f);
	m_model2.textureFilter = gl4::MagFilter::Linear;
	m_model2.material.normalTexture = TextureManager::GetTexture("CoreData/textures/normal01.tga");

	m_model3.mesh = LoadAssimpMesh("ExampleData/mesh/stall/stall.obj");
	m_model3.scale = glm::vec3(0.3f);
	m_model3.position = glm::vec3(4.0f, -0.6f, 0.0f);
	m_model3.textureFilter = gl4::MagFilter::Linear;
	m_model3.material.diffuseTexture = TextureManager::GetTexture("ExampleData/mesh/stall/stallTexture.png", false);
	m_model3.material.normalTexture = TextureManager::GetTexture("CoreData/textures/normal01.tga");

	m_model4.mesh = LoadAssimpMesh("ExampleData/mesh/cube.obj");
	m_model4.position = glm::vec3(-2.0f, 0.0f, 2.0f);
	m_model4.textureFilter = gl4::MagFilter::Linear;
	m_model4.material.diffuseTexture = TextureManager::GetTexture("ExampleData/textures/wood.jpg");
	m_model4.material.normalTexture = TextureManager::GetTexture("CoreData/textures/normal01.tga");

	return true;
}
//=============================================================================
void World::Close()
{
	delete m_model1.mesh;
	delete m_model2.mesh;
	delete m_model3.mesh;
	delete m_model4.mesh;
}
//=============================================================================