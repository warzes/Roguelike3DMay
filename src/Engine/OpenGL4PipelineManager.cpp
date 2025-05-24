#include "stdafx.h"
#include "OpenGL4PipelineManager.h"
#include "OpenGL4Shader.h"
#include "OpenGL4Context.h"
#include "Log.h"
//=============================================================================
namespace
{
	std::unordered_map<GLuint, std::shared_ptr<const gl4::detail::GraphicsPipelineInfoOwning>> gGraphicsPipelines;
	std::unordered_map<GLuint, std::shared_ptr<const gl4::detail::ComputePipelineInfoOwning>> gComputePipelines;

	gl4::detail::GraphicsPipelineInfoOwning makePipelineInfoOwning(const gl4::GraphicsPipelineInfo& info)
	{
		return gl4::detail::GraphicsPipelineInfoOwning{
			.name = std::string(info.name),
			.inputAssemblyState = info.inputAssemblyState,
			.vertexInputState =
				{
					{
						info.vertexInputState.vertexBindingDescriptions.begin(),
						info.vertexInputState.vertexBindingDescriptions.end(),
					},
				},
			.tessellationState = info.tessellationState,
			.rasterizationState = info.rasterizationState,
			.multisampleState = info.multisampleState,
			.depthState = info.depthState,
			.stencilState = info.stencilState,
			.colorBlendState{
				.logicOpEnable = info.colorBlendState.logicOpEnable,
				.logicOp = info.colorBlendState.logicOp,
				.attachments = {info.colorBlendState.attachments.begin(), info.colorBlendState.attachments.end()},
				.blendConstants =
				{
					info.colorBlendState.blendConstants[0],
					info.colorBlendState.blendConstants[1],
					info.colorBlendState.blendConstants[2],
					info.colorBlendState.blendConstants[3],
				},
			},
		};
	}

	bool linkProgram(GLuint program, std::string& outInfoLog)
	{
		glLinkProgram(program);

		GLint success{};
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLint length = 512;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
			outInfoLog.resize(static_cast<size_t>(length + 1), '\0');
			glGetProgramInfoLog(program, length, nullptr, outInfoLog.data());
			return false;
		}

		return true;
	}

	std::vector<std::pair<std::string, uint32_t>> reflectProgram(GLuint program, GLenum interface)
	{
		GLint numActiveResources{};
		glGetProgramInterfaceiv(program, interface, GL_ACTIVE_RESOURCES, &numActiveResources);

		GLint maxNameLength{};
		glGetProgramInterfaceiv(program, interface, GL_MAX_NAME_LENGTH, &maxNameLength);

		auto reflected = std::vector<std::pair<std::string, uint32_t>>(static_cast<size_t>(numActiveResources), { std::string(static_cast<size_t>(maxNameLength), '\0'), 0 });

		for (GLint i = 0; i < numActiveResources; i++)
		{
			// Reflect the name of the resource
			glGetProgramResourceName(program,
				interface,
				i,
				static_cast<GLsizei>(reflected[i].first.size()),
				nullptr,
				reflected[i].first.data());

			// Reflect the value or binding of the resource (for samplers and images, these are essentially bindings as well)
			if (interface == GL_UNIFORM)
			{
				auto location = glGetProgramResourceLocation(program, interface, reflected[i].first.c_str());

				// Variables inside uniform blocks will also be iterated here. Their location will be -1, so we can skip them this way.
				if (location != -1)
				{
					// Wrong if the uniform isn't an integer (e.g., sampler or image). OK, default-block uniforms are not supported otherwise.
					GLint binding{ -1 };
					glGetUniformiv(program, location, &binding);
					reflected[i].second = static_cast<uint32_t>(binding);
				}
				else
				{
					reflected[i].first = "";
				}
			}
			else if (interface == GL_UNIFORM_BLOCK || interface == GL_SHADER_STORAGE_BLOCK)
			{
				GLint binding{ -1 };
				constexpr GLenum property = GL_BUFFER_BINDING;
				glGetProgramResourceiv(program, interface, i, 1, &property, 1, nullptr, &binding);
				reflected[i].second = static_cast<uint32_t>(binding);
			}
			else
			{
				assert(0);
			}
		}

		auto it = std::remove_if(reflected.begin(), reflected.end(), [](const auto& pair) { return pair.first.empty(); });
		reflected.erase(it, reflected.end());

		return reflected;
	}
}
//=============================================================================
uint64_t gl4::detail::CompileGraphicsPipelineInternal(const GraphicsPipelineInfo& info)
{
	assert(info.vertexShader && "A graphics pipeline must at least have a vertex shader");
	if (info.tessellationControlShader || info.tessellationEvaluationShader)
	{
		assert(info.tessellationControlShader && info.tessellationEvaluationShader && "Either both or neither tessellation shader can be present");
	}
	GLuint program = glCreateProgram();
	glAttachShader(program, info.vertexShader->Handle());
	if (info.fragmentShader)
	{
		glAttachShader(program, info.fragmentShader->Handle());
	}

	if (info.tessellationControlShader)
	{
		glAttachShader(program, info.tessellationControlShader->Handle());
	}

	if (info.tessellationEvaluationShader)
	{
		glAttachShader(program, info.tessellationEvaluationShader->Handle());
	}

	std::string infolog;
	if (!linkProgram(program, infolog))
	{
		glDeleteProgram(program);
		Error("Failed to compile graphics pipeline.\n" + infolog);
		return 0;
	}

	if (!info.name.empty())
	{
		glObjectLabel(GL_PROGRAM, program, static_cast<GLsizei>(info.name.length()), info.name.data());
	}

	auto owning = makePipelineInfoOwning(info);
	owning.uniformBlocks = reflectProgram(program, GL_UNIFORM_BLOCK);
	owning.storageBlocks = reflectProgram(program, GL_SHADER_STORAGE_BLOCK);
	owning.samplersAndImages = reflectProgram(program, GL_UNIFORM);

	gGraphicsPipelines.insert({ program, std::make_shared<const GraphicsPipelineInfoOwning>(std::move(owning)) });
	return program;
}
//=============================================================================
std::shared_ptr<const gl4::detail::GraphicsPipelineInfoOwning> gl4::detail::GetGraphicsPipelineInternal(uint64_t pipeline)
{
	if (auto it = gGraphicsPipelines.find(static_cast<GLuint>(pipeline)); it != gGraphicsPipelines.end())
	{
		return it->second;
	}
	return nullptr;
}
//=============================================================================
void gl4::detail::DestroyGraphicsPipelineInternal(uint64_t pipeline)
{
	auto it = gGraphicsPipelines.find(static_cast<GLuint>(pipeline));
	if (it == gGraphicsPipelines.end())
	{
		// Tried to delete a nonexistent pipeline.
		assert(0);
		return;
	}

	glDeleteProgram(static_cast<GLuint>(pipeline));
	gGraphicsPipelines.erase(it);
}
//=============================================================================
uint64_t gl4::detail::CompileComputePipelineInternal(const ComputePipelineInfo& info)
{
	assert(info.shader);
	GLuint program = glCreateProgram();
	glAttachShader(program, info.shader->Handle());

	std::string infolog;
	if (!linkProgram(program, infolog))
	{
		glDeleteProgram(program);
		Error("Failed to compile compute pipeline.\n" + infolog);
		return 0;
	}

	if (!info.name.empty())
	{
		glObjectLabel(GL_PROGRAM, program, static_cast<GLsizei>(info.name.length()), info.name.data());
	}

	GLint workgroupSize[3];
	glGetProgramiv(program, GL_COMPUTE_WORK_GROUP_SIZE, workgroupSize);

	assert(workgroupSize[0] <= gContext.properties.limits.maxComputeWorkGroupSize[0] &&
		workgroupSize[1] <= gContext.properties.limits.maxComputeWorkGroupSize[1] &&
		workgroupSize[2] <= gContext.properties.limits.maxComputeWorkGroupSize[2]);
	assert(workgroupSize[0] * workgroupSize[1] * workgroupSize[2] <= gContext.properties.limits.maxComputeWorkGroupInvocations);

	auto owning = ComputePipelineInfoOwning{ .name = std::string(info.name) };
	owning.uniformBlocks = reflectProgram(program, GL_UNIFORM_BLOCK);
	owning.storageBlocks = reflectProgram(program, GL_SHADER_STORAGE_BLOCK);
	owning.samplersAndImages = reflectProgram(program, GL_UNIFORM);

	owning.workgroupSize.width = static_cast<uint32_t>(workgroupSize[0]);
	owning.workgroupSize.height = static_cast<uint32_t>(workgroupSize[1]);
	owning.workgroupSize.depth = static_cast<uint32_t>(workgroupSize[2]);

	gComputePipelines.insert({ program, std::make_shared<const ComputePipelineInfoOwning>(std::move(owning)) });
	return program;
}
//=============================================================================
std::shared_ptr<const gl4::detail::ComputePipelineInfoOwning> gl4::detail::GetComputePipelineInternal(uint64_t pipeline)
{
	if (auto it = gComputePipelines.find(static_cast<GLuint>(pipeline)); it != gComputePipelines.end())
	{
		return it->second;
	}
	return nullptr;
}
//=============================================================================
void gl4::detail::DestroyComputePipelineInternal(uint64_t pipeline)
{
	auto it = gComputePipelines.find(static_cast<GLuint>(pipeline));
	if (it == gComputePipelines.end())
	{
		// Tried to delete a nonexistent pipeline.
		assert(0);
		return;
	}

	glDeleteProgram(static_cast<GLuint>(pipeline));
	gComputePipelines.erase(it);
}
//=============================================================================