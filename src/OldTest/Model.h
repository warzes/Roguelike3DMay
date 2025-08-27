#pragma once

#include "Material.h"

// TODO: это для того чтобы создать модель из нескольких мешей, с возможностью хранить данные в одном общем вершинном буфере
struct MeshDescriptor final
{
	std::vector<std::vector<MeshVertex>> vertices;
	std::vector<std::vector<uint32_t>> indices;
	std::vector<std::string> materials;
};