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
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename.c_str(), ASSIMP_LOAD_FLAGS);
	if (!scene || !scene->HasMeshes())
	{
		Fatal("Not load mesh: " + filename + "\n\tError: " + importer.GetErrorString());
		return nullptr;
	}

	const aiMesh* mesh = scene->mMeshes[0];

	std::vector<MeshVertex> vertices(mesh->mNumVertices);
	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		MeshVertex& v = vertices[i];

		v.position.x = mesh->mVertices[i].x;
		v.position.y = mesh->mVertices[i].y;
		v.position.z = mesh->mVertices[i].z;

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

		//if (mesh->HasNormals())
		{
			v.normal.x = mesh->mNormals[i].x;
			v.normal.y = mesh->mNormals[i].y;
			v.normal.z = mesh->mNormals[i].z;
		}

		//if (mesh->HasTextureCoords(0))
		{
			v.uv.x = mesh->mTextureCoords[0][i].x;
			v.uv.y = mesh->mTextureCoords[0][i].y;
		}

		//if (mesh->HasTangentsAndBitangents())
		{
			v.tangent.x = mesh->mTangents[i].x;
			v.tangent.y = mesh->mTangents[i].y;
			v.tangent.z = mesh->mTangents[i].z;
		}
	}

	// Fill face indices
	std::vector<uint32_t> indices;
	indices.reserve(mesh->mNumFaces * 3);
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		// Assume the model has only triangles.
		indices.emplace_back(mesh->mFaces[i].mIndices[0]);
		indices.emplace_back(mesh->mFaces[i].mIndices[1]);
		indices.emplace_back(mesh->mFaces[i].mIndices[2]);
	}

	return LoadDataMesh(vertices, indices);
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