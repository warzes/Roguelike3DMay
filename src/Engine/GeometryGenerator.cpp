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
void buildPlane(MeshCreateInfo& meshInfo, int& numberOfVertices, int u, int v, int w, float udir, float vdir, float width, float height, float depth, float gridX, float gridY, float materialIndex)
{
	float segmentWidth = width / gridX;
	float segmentHeight = height / gridY;

	float widthHalf = width / 2.0f;
	float heightHalf = height / 2.0f;
	float depthHalf = depth / 2.0f;

	float gridX1 = gridX + 1.0f;
	float gridY1 = gridY + 1.0f;

	float vertexCounter = 0.0f;
	float groupCount = 0.0f;

	glm::vec3 vector = glm::vec3(0.0f, 0.0f, 0.0f);

	MeshVertex vertex;

	for (unsigned int iy = 0; iy < gridY1; iy++)
	{
		float y = iy * segmentHeight - heightHalf;
		for (unsigned int ix = 0; ix < gridX1; ix++)
		{
			float x = ix * segmentWidth - widthHalf;

			// position
			vector[u] = x * udir;
			vector[v] = y * vdir;
			vector[w] = depthHalf;
			vertex.position = glm::vec3(vector.x, vector.y, vector.z);

			// normals
			vector[u] = 0;
			vector[v] = 0;
			vector[w] = depth > 0 ? 1 : -1;
			vertex.normal = glm::vec3(vector.x, vector.y, vector.z);

			// uvs
			vertex.texCoord = glm::vec2(ix / gridX, 1 - (iy / gridY));

			meshInfo.vertices.push_back(vertex);

			// counters
			vertexCounter += 1.0;
		}
	}

	// indices
	for (unsigned int iy = 0; iy < gridY; iy++)
	{
		for (unsigned int ix = 0; ix < gridX; ix++)
		{
			float a = numberOfVertices + ix + gridX1 * iy;
			float b = numberOfVertices + ix + gridX1 * (iy + 1);
			float c = numberOfVertices + (ix + 1) + gridX1 * (iy + 1);
			float d = numberOfVertices + (ix + 1) + gridX1 * iy;

			meshInfo.indices.push_back(a);
			meshInfo.indices.push_back(b);
			meshInfo.indices.push_back(d);

			meshInfo.indices.push_back(b);
			meshInfo.indices.push_back(c);
			meshInfo.indices.push_back(d);
		}
	}
	numberOfVertices += vertexCounter;
}
//=============================================================================
MeshCreateInfo GeometryGenerator::CreateBox(float width, float height, float depth, float widthSeg, float heightSeg, float depthSeg)
{
	MeshCreateInfo meshInfo;

	float widthSegments = glm::floor(widthSeg);
	float heightSegments = glm::floor(heightSeg);
	float depthSegments = glm::floor(depthSeg);

	int numberOfVertices = 0;

	buildPlane(meshInfo, numberOfVertices, 2, 1, 0, -1, -1, depth, height, width, depthSegments, heightSegments, 0); // px
	buildPlane(meshInfo, numberOfVertices, 2, 1, 0, 1, -1, depth, height, -width, depthSegments, heightSegments, 1); // nx

	buildPlane(meshInfo, numberOfVertices, 0, 2, 1, 1, 1, width, depth, height, widthSegments, depthSegments, 2); // py
	buildPlane(meshInfo, numberOfVertices, 0, 2, 1, 1, -1, width, depth, -height, widthSegments, depthSegments, 3); // ny

	buildPlane(meshInfo, numberOfVertices, 0, 1, 2, 1, -1, width, height, depth, widthSegments, heightSegments, 4); // pz
	buildPlane(meshInfo, numberOfVertices, 0, 1, 2, -1, -1, width, height, -depth, widthSegments, heightSegments, 5); // nz

	return meshInfo;
}
//=============================================================================