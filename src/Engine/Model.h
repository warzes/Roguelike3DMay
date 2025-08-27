#pragma once

#include "Mesh.h"

class Model final
{
public:

private:
	std::vector<Mesh*> m_meshes;
	AABB               m_aabb;
};
