#pragma once

#include "Mesh.h"

// TODO: это для того чтобы создать модель из нескольких мешей, с возможностью хранить данные в одном общем вершинном буфере
struct MeshDescriptor final
{
	std::vector<std::vector<MeshVertex>> vertices;
	std::vector<std::vector<uint32_t>> indices;
	std::vector<std::string> materials;
};

// TODO: сделать возможность хранить буфер вершин/индексов в Model, а здесь хранить смещения в буфере
class Model final
{
public:
	bool Load(const std::string& path, MaterialManager& materialManager);

private:
	std::vector<Mesh> m_meshes;
	AABB m_bbox;
};