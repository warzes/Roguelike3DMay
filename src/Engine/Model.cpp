#include "stdafx.h"
#include "Model.h"
#include "FileUtils.h"
#include "Log.h"
#include "TextureManager.h"
//=============================================================================
#define ASSIMP_LOAD_FLAGS (aiProcess_JoinIdenticalVertices |    \
                           aiProcess_Triangulate |              \
                           aiProcess_GenSmoothNormals |         \
                           aiProcess_LimitBoneWeights |         \
                           aiProcess_SplitLargeMeshes |         \
                           aiProcess_ImproveCacheLocality |     \
                           aiProcess_RemoveRedundantMaterials | \
                           aiProcess_FindDegenerates |          \
                           aiProcess_FindInvalidData |          \
                           aiProcess_GenUVCoords |              \
                           aiProcess_FlipUVs |                  \
                           aiProcess_MakeLeftHanded |           \
                           aiProcess_CalcTangentSpace |         \
                           aiProcess_SortByPType)
//=============================================================================
Model::~Model()
{
	Free();
}
//=============================================================================
bool Model::Load(const std::string& fileName, std::optional<glm::mat4> modelTransformMatrix)
{
	Free();

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(fileName.c_str(), ASSIMP_LOAD_FLAGS);
	if (!scene || !scene->HasMeshes() || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		Fatal("Not load mesh: " + fileName + "\n\tError: " + importer.GetErrorString());
		return false;
	}

	std::string directory = io::GetFileDirectory(fileName);
	processNode(scene, scene->mRootNode, directory, modelTransformMatrix);

	computeAABB();

	return true;
}
//=============================================================================
void Model::Create(const MeshCreateInfo& ci)
{
	Free();
	m_meshes.emplace_back(new Mesh(ci.vertices, ci.indices, ci.material));
	computeAABB();
}
//=============================================================================
void Model::Create(const std::vector<MeshCreateInfo>& meshes)
{
	Free();
	for (size_t i = 0; i < meshes.size(); i++)
	{
		m_meshes.emplace_back(new Mesh(meshes[i].vertices, meshes[i].indices, meshes[i].material));
	}
	computeAABB();
}
//=============================================================================
void Model::Free()
{
	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		delete m_meshes[i]->GetMaterial();
		delete m_meshes[i];
	}
	m_meshes.clear();
}
//=============================================================================
void Model::DrawSubMesh(size_t id, std::optional<gl::Sampler> sampler)
{
	if (id < m_meshes.size())
		m_meshes[id]->Bind(sampler);
}
//=============================================================================
void Model::Draw(std::optional<gl::Sampler> sampler)
{
	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		m_meshes[i]->Bind(sampler);
	}
}
//=============================================================================
void Model::processNode(const aiScene* scene, aiNode* node, std::string_view directory, std::optional<glm::mat4> modelTransformMatrix)
{
	for (unsigned i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_meshes.emplace_back(processMesh(scene, mesh, directory, modelTransformMatrix));
	}

	for (unsigned i = 0; i < node->mNumChildren; i++)
	{
		processNode(scene, node->mChildren[i], directory, modelTransformMatrix);
	}
}
//=============================================================================
Mesh* Model::processMesh(const aiScene* scene, struct aiMesh* mesh, std::string_view directory, std::optional<glm::mat4> modelTransformMatrix)
{
	std::vector<MeshVertex> vertices(mesh->mNumVertices);

	std::vector<uint32_t> indices;
	indices.reserve(mesh->mNumFaces * 3);

	PhongMaterial* meshMaterial{ nullptr };

	// Process vertices
	for (unsigned i = 0; i < mesh->mNumVertices; i++)
	{
		MeshVertex& v = vertices[i];

		v.position.x = mesh->mVertices[i].x;
		v.position.y = mesh->mVertices[i].y;
		v.position.z = mesh->mVertices[i].z;
		if (modelTransformMatrix.has_value())
		{
			v.position = glm::vec3(modelTransformMatrix.value() * glm::vec4(v.position, 1.0f));
		}

		if (mesh->HasVertexColors(0))
		{
			v.color.x = mesh->mColors[0][i].r;
			v.color.y = mesh->mColors[0][i].g;
			v.color.z = mesh->mColors[0][i].b;
		}
		else
		{
			v.color = glm::vec3(1.0f);
		}

		if (mesh->HasNormals())
		{
			v.normal.x = mesh->mNormals[i].x;
			v.normal.y = mesh->mNormals[i].y;
			v.normal.z = mesh->mNormals[i].z;
			if (modelTransformMatrix.has_value())
			{
				const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelTransformMatrix.value())));

				v.normal = glm::normalize(normalMatrix * v.normal);
			}
		}

		if (mesh->HasTextureCoords(0))
		{
			v.texCoord.x = mesh->mTextureCoords[0][i].x;
			v.texCoord.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			v.texCoord = glm::vec2(0.0f);
		}

		if (mesh->HasTangentsAndBitangents())
		{
			v.tangent.x = mesh->mTangents[i].x;
			v.tangent.y = mesh->mTangents[i].y;
			v.tangent.z = mesh->mTangents[i].z;
		}
		else
			v.tangent = glm::vec3(0.0f);
	}

	// Process indices
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		// Assume the model has only triangles.
		indices.emplace_back(mesh->mFaces[i].mIndices[0]);
		indices.emplace_back(mesh->mFaces[i].mIndices[1]);
		indices.emplace_back(mesh->mFaces[i].mIndices[2]);
	}

	// Process material
	{
		// TODO: по одной текстуре грузится, а тут есть возможность нескольих текстур
		meshMaterial = new PhongMaterial();

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		for (unsigned i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++)
		{
			aiString str;
			material->GetTexture(aiTextureType_DIFFUSE, i, &str);

			std::string path = directory.data();
			path += str.data;
			meshMaterial->diffuseTexture = TextureManager::GetTexture(path, gl::ColorSpace::sRGB);
		}

		for (unsigned i = 0; i < material->GetTextureCount(aiTextureType_NORMALS); i++)
		{
			aiString str;
			material->GetTexture(aiTextureType_NORMALS, i, &str);

			std::string path = directory.data();
			path += str.data;
			meshMaterial->normalTexture = TextureManager::GetTexture(path);
		}

		for (unsigned i = 0; i < material->GetTextureCount(aiTextureType_SPECULAR); i++)
		{
			aiString str;
			material->GetTexture(aiTextureType_SPECULAR, i, &str);

			std::string path = directory.data();
			path += str.data;
			meshMaterial->specularTexture = TextureManager::GetTexture(path);
		}
	}

	return new Mesh(vertices, indices, meshMaterial);
}
//=============================================================================
void Model::computeAABB()
{
	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		m_aabb.CombineAABB(m_meshes[i]->GetAABB());
	}
}
//=============================================================================