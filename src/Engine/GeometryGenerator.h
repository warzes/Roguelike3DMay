#pragma once

#include "Model.h"

namespace GeometryGenerator
{
	MeshCreateInfo CreatePlane(float width = 1.0f, float height = 1.0f, float wSegment = 1.0f, float hSegment = 1.0f);
	MeshCreateInfo CreateBox(float width = 1.0f, float height = 1.0f, float depth = 1.0, float widthSegments = 1.0f, float heightSegments = 1.0f, float depthSegments = 1.0f);
	MeshCreateInfo CreateSphere(float radius = 1.0f, int slices = 32, int stacks = 16);
} // namespace GeometryGenerator