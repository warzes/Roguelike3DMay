#pragma once

const char* shaderQuadCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;

layout(location = 0) out vec2 fragTexCoord;

void main()
{
	fragTexCoord = vertexTexCoord;
	gl_Position  = vec4(vertexPosition, 1.0);
}
)";

const char* shaderQuadCodeFragment = R"(
#version 460 core

layout(location = 0) in vec2 fragTexCoord;

layout(binding = 0) uniform sampler2D diffuseTex;

layout(location = 0) out vec4 outputColor;

void main()
{
	outputColor = texture(diffuseTex, fragTexCoord);
}
)";