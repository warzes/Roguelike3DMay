#pragma once

#include "Mesh.h"
#include "AABB.h"

class Model final
{
public:

private:
	std::vector<Mesh> m_meshes;
	AABB m_bbox;
};