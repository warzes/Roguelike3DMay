#include "stdafx.h"
#include "GameMeshObject.h"
//=============================================================================
GameMeshObject::GameMeshObject()
{
	m_type = GameObjectType::Model;
}
//=============================================================================
bool GameMeshObject::Load(const std::string& fileName, std::optional<glm::mat4> modelTransformMatrix)
{
	return m_model.Load(fileName, modelTransformMatrix);
}
//=============================================================================
void GameMeshObject::Create(const MeshCreateInfo& meshCreateInfo)
{
	m_model.Create(meshCreateInfo);
}
//=============================================================================
void GameMeshObject::Create(const std::vector<MeshCreateInfo>& meshes)
{
	m_model.Create(meshes);
}
//=============================================================================
void GameMeshObject::Free()
{
	m_model.Free();
}
//=============================================================================
void GameMeshObject::Draw(std::optional<gl::Sampler> sampler)
{
	m_model.Draw(sampler);
}
//=============================================================================