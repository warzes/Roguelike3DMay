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
void buildBoxPlane(MeshCreateInfo& meshInfo, int& numberOfVertices, int u, int v, int w, float udir, float vdir, float width, float height, float depth, float gridX, float gridY, float materialIndex)
{
	float segmentWidth = width / gridX;
	float segmentHeight = height / gridY;

	float widthHalf = width / 2.0f;
	float heightHalf = height / 2.0f;
	float depthHalf = depth / 2.0f;

	float gridX1 = gridX + 1.0f;
	float gridY1 = gridY + 1.0f;

	MeshVertex vertex;

	for (unsigned int iy = 0; iy < gridY1; iy++)
	{
		float y = iy * segmentHeight - heightHalf;
		for (unsigned int ix = 0; ix < gridX1; ix++)
		{
			float x = ix * segmentWidth - widthHalf;

			// position
			vertex.position[u] = x * udir;
			vertex.position[v] = y * vdir;
			vertex.position[w] = depthHalf;

			// normals
			vertex.normal[u] = 0.0f;
			vertex.normal[v] = 0.0f;
			vertex.normal[w] = depth > 0 ? 1.0f : -1.0f;

			// uvs
			vertex.texCoord = glm::vec2(static_cast<float>(ix) / gridX, 1.0f - static_cast<float>(iy) / gridY);

			meshInfo.vertices.push_back(vertex);
		}
	}

	// indices
	for (unsigned int iy = 0; iy < gridY; iy++)
	{
		for (unsigned int ix = 0; ix < gridX; ix++)
		{
			unsigned int a = numberOfVertices + ix + iy * gridX1;
			unsigned int b = numberOfVertices + ix + (iy + 1) * gridX1;
			unsigned int c = numberOfVertices + (ix + 1) + (iy + 1) * gridX1;
			unsigned int d = numberOfVertices + (ix + 1) + iy * gridX1;

			meshInfo.indices.push_back(a);
			meshInfo.indices.push_back(d);
			meshInfo.indices.push_back(b);

			meshInfo.indices.push_back(b);
			meshInfo.indices.push_back(d);
			meshInfo.indices.push_back(c);
		}
	}
	numberOfVertices += static_cast<int>(gridX1 * gridY1);
}
//=============================================================================
MeshCreateInfo GeometryGenerator::CreateBox(float width, float height, float depth, float widthSeg, float heightSeg, float depthSeg)
{
	MeshCreateInfo meshInfo;

	int widthSegments = std::max(1, static_cast<int>(std::floor(widthSeg)));
	int heightSegments = std::max(1, static_cast<int>(std::floor(heightSeg)));
	int depthSegments = std::max(1, static_cast<int>(std::floor(depthSeg)));

	int numberOfVertices = 0;

	// Плоскости: +X, -X, +Y, -Y, +Z, -Z
	buildBoxPlane(meshInfo, numberOfVertices, 2, 1, 0, -1, -1, depth, height, width, depthSegments, heightSegments, 0); // +X
	buildBoxPlane(meshInfo, numberOfVertices, 2, 1, 0, 1, -1, depth, height, -width, depthSegments, heightSegments, 1); // -X

	buildBoxPlane(meshInfo, numberOfVertices, 0, 2, 1, 1, 1, width, depth, height, widthSegments, depthSegments, 2); // +Y
	buildBoxPlane(meshInfo, numberOfVertices, 0, 2, 1, 1, -1, width, depth, -height, widthSegments, depthSegments, 3); // -Y

	buildBoxPlane(meshInfo, numberOfVertices, 0, 1, 2, 1, -1, width, height, depth, widthSegments, heightSegments, 4); // +Z
	buildBoxPlane(meshInfo, numberOfVertices, 0, 1, 2, -1, -1, width, height, -depth, widthSegments, heightSegments, 5); // -Z

	return meshInfo;
}
//=============================================================================