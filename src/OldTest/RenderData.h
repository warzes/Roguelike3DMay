#pragma once

struct RenderTarget final
{
	GLuint fbo{ 0 };
	GLuint colorTarget{ 0 };
	GLuint depthStencilTarget{ 0 };
	uint16_t width{ 0 };
	uint16_t height{ 0 };
	int samples{ 0 };

	glm::vec3 clearColor = glm::vec3(1.0f);
};