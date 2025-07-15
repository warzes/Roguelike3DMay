#include "stdafx.h"
#include "GameModel.h"
//=============================================================================
Mesh* LoadDataMesh(const std::vector<MeshVertex>& vertex, const std::vector<uint32_t>& indices)
{
	return new Mesh(vertex, indices, std::nullopt);
}
//=============================================================================
Mesh* LoadAssimpMesh(const std::string& filename)
{
	const aiScene* scene = aiImportFile(filename.c_str(), ASSIMP_LOAD_FLAGS);
	if (!scene || !scene->HasMeshes())
	{
		Fatal("Not load mesh: " + filename);
		return nullptr;
	}

	const aiMesh* mesh = scene->mMeshes[0];
	std::vector<MeshVertex> mv(mesh->mNumVertices);

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		MeshVertex& nv = mv[i];

		nv.position.x = mesh->mVertices[i].x;
		nv.position.y = mesh->mVertices[i].y;
		nv.position.z = mesh->mVertices[i].z;

		//if (mesh->HasNormals())
		{
			nv.normal.x = mesh->mNormals[i].x;
			nv.normal.y = mesh->mNormals[i].y;
			nv.normal.z = mesh->mNormals[i].z;
		}

		//if (mesh->HasTextureCoords(0))
		{
			nv.uv.x = mesh->mTextureCoords[0][i].x;
			nv.uv.y = mesh->mTextureCoords[0][i].y;
			const auto tc = mesh->mTextureCoords[0][i];
		}

		//if (mesh->HasTangentsAndBitangents())
		{
			nv.tangent.x = mesh->mTangents[i].x;
			nv.tangent.y = mesh->mTangents[i].y;
			nv.tangent.z = mesh->mTangents[i].z;
		}
	}
	//miv.resize(mesh->mNumFaces * 3);
	std::vector<uint32_t> miv;
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			miv.emplace_back(mesh->mFaces[i].mIndices[j]);
		}
	}

	aiReleaseImport(scene);
	return LoadDataMesh(mv, miv);
}
//=============================================================================
glm::mat4 GameModel::GetModelMat() const
{
	// TODO: сделать кеширование, если модель не двигается, то возвращать кеш

	glm::mat4 model = glm::mat4(1.0f);

	// Перемещение
	model = glm::translate(model, position);

	// Вращение вокруг осей X, Y, Z (в градусах)
	model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));

	// Масштабирование
	model = glm::scale(model, scale);

	return model;
}
//=============================================================================