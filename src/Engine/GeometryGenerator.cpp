#include "stdafx.h"
#include "GeometryGenerator.h"
//=============================================================================
MeshCreateInfo GeometryGenerator::CreatePlane(float width, float height, float wSegment, float hSegment)
{
	MeshCreateInfo meshInfo;

	float width_half = width / 2.0f;
	float height_half = height / 2.0f;

	float gridX1 = wSegment + 1.0f;
	float gridY1 = hSegment + 1.0f;

	float segment_width = width / wSegment;
	float segment_height = height / hSegment;

	MeshVertex vertex;

	// generate Position Normal TexCoords
	for (int iy = 0; iy < gridY1; iy++)
	{
		float y = iy * segment_height - height_half;

		for (int ix = 0; ix < gridX1; ix++)
		{
			float x = ix * segment_width - height_half;
			vertex.position = glm::vec3(x, 0.0f, -y);
			vertex.color = glm::vec3(1.0f);
			vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
			vertex.texCoord = glm::vec2(ix / wSegment, 1.0f - (iy / hSegment));
			vertex.tangent = glm::vec3(0.0f);
			meshInfo.vertices.push_back(vertex);
		}
	}
	// generate indices
	for (int iy = 0; iy < hSegment; iy++)
	{
		for (int ix = 0; ix < wSegment; ix++)
		{
			float a = ix + gridX1 * iy;
			float b = ix + gridX1 * (iy + 1);
			float c = (ix + 1) + gridX1 * (iy + 1);
			float d = (ix + 1) + gridX1 * iy;
			meshInfo.indices.push_back(a);
			meshInfo.indices.push_back(b);
			meshInfo.indices.push_back(d);
			meshInfo.indices.push_back(b);
			meshInfo.indices.push_back(c);
			meshInfo.indices.push_back(d);
		}
	}

	return meshInfo;
}
//=============================================================================