#pragma once

struct ObjectMeshed
{
	Transform transform;
	std::vector<Mesh*> meshes;
};

struct ObjectBatched
{
	Transform transform;
	//std::vector<MeshInfo> meshes;
};

struct alignas(16) ObjectUniforms // sent to GPU
{
	glm::mat4 modelMatrix{};
	//glm::mat4 normalMatrix{};
	uint32_t materialIndex{};
};