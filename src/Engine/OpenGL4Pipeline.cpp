#include "stdafx.h"
#include "OpenGL4Pipeline.h"
#include "OpenGL4Shader.h"
#include "OpenGL4DeviceProperties.h"
#include "Log.h"
//=============================================================================
std::unordered_map<GLuint, std::shared_ptr<const gl::detail::GraphicsPipelineInfoOwning>> gGraphicsPipelines;
std::unordered_map<GLuint, std::shared_ptr<const gl::detail::ComputePipelineInfoOwning>>  gComputePipelines;
//=============================================================================
std::shared_ptr<const gl::detail::GraphicsPipelineInfoOwning> gl::detail::GetGraphicsPipelineInternal(uint64_t pipeline)
{
	if (auto it = gGraphicsPipelines.find(static_cast<GLuint>(pipeline)); it != gGraphicsPipelines.end())
	{
		return it->second;
	}
	return nullptr;
}
//=============================================================================
std::shared_ptr<const gl::detail::ComputePipelineInfoOwning> gl::detail::GetComputePipelineInternal(uint64_t pipeline)
{
	if (auto it = gComputePipelines.find(static_cast<GLuint>(pipeline)); it != gComputePipelines.end())
	{
		return it->second;
	}
	return nullptr;
}
//=============================================================================
inline bool linkProgram(GLuint program, std::string& outInfoLog)
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
//=============================================================================
inline gl::detail::GraphicsPipelineInfoOwning makePipelineInfoOwning(const gl::GraphicsPipelineInfo& info)
{
	return gl::detail::GraphicsPipelineInfoOwning{
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
		.multisamplingState = info.multisamplingState,
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
//=============================================================================
inline std::vector<std::pair<std::string, uint32_t>> reflectProgram(GLuint program, GLenum interface)
{
	GLint numActiveResources{};
	glGetProgramInterfaceiv(program, interface, GL_ACTIVE_RESOURCES, &numActiveResources);

	GLint maxNameLength{};
	glGetProgramInterfaceiv(program, interface, GL_MAX_NAME_LENGTH, &maxNameLength);

	auto reflected = std::vector<std::pair<std::string, uint32_t>>(static_cast<size_t>(numActiveResources), { std::string(static_cast<size_t>(maxNameLength), '\0'), 0 });

	for (size_t i = 0; i < static_cast<size_t>(numActiveResources); i++)
	{
		// Reflect the name of the resource
		glGetProgramResourceName(program,
			interface,
			static_cast<GLuint>(i),
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
			glGetProgramResourceiv(program, interface, static_cast<GLuint>(i), 1, &property, 1, nullptr, &binding);
			reflected[i].second = static_cast<uint32_t>(binding);
		}
		else
		{
			std::unreachable();
		}
	}

	auto it = std::remove_if(reflected.begin(), reflected.end(), [](const auto& pair) { return pair.first.empty(); });
	reflected.erase(it, reflected.end());

	return reflected;
}
//=============================================================================
gl::GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineInfo& info)
{
	assert(info.vertexShader && "A graphics pipeline must at least have a vertex shader");
	if (info.tessellationControlShader || info.tessellationEvaluationShader)
	{
		assert(info.tessellationControlShader && info.tessellationEvaluationShader && "Either both or neither tessellation shader can be present");
	}
	GLuint program = glCreateProgram();
	if (!program) return;

	glAttachShader(program, info.vertexShader->Handle());
	if (info.fragmentShader)
		glAttachShader(program, info.fragmentShader->Handle());

	if (info.tessellationControlShader)
		glAttachShader(program, info.tessellationControlShader->Handle());

	if (info.tessellationEvaluationShader)
		glAttachShader(program, info.tessellationEvaluationShader->Handle());

	std::string infoLog;
	if (!linkProgram(program, infoLog))
	{
		glDeleteProgram(program);
		Error("Failed to compile graphics pipeline.\n" + infoLog);
		return;
	}

	if (!info.name.empty())
		glObjectLabel(GL_PROGRAM, program, static_cast<GLsizei>(info.name.length()), info.name.data());

	auto owning = makePipelineInfoOwning(info);
	owning.uniformBlocks     = reflectProgram(program, GL_UNIFORM_BLOCK);
	owning.storageBlocks     = reflectProgram(program, GL_SHADER_STORAGE_BLOCK);
	owning.samplersAndImages = reflectProgram(program, GL_UNIFORM);

	gGraphicsPipelines.insert({ program, std::make_shared<const gl::detail::GraphicsPipelineInfoOwning>(std::move(owning)) });
	m_id = program;

	Debug("Created Graphics Program with handle " + std::to_string(m_id));
}
//=============================================================================
gl::GraphicsPipeline::~GraphicsPipeline()
{
	if (m_id != 0)
	{
		Debug("Destroyed Graphics Program with handle " + std::to_string(m_id));
		auto it = gGraphicsPipelines.find(static_cast<GLuint>(m_id));
		if (it == gGraphicsPipelines.end())
		{
			std::unreachable();
			return;
		}

		glDeleteProgram(static_cast<GLuint>(m_id));
		gGraphicsPipelines.erase(it);
	}
}
//=============================================================================
gl::GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& old) noexcept : m_id(std::exchange(old.m_id, 0)) {}
//=============================================================================
gl::GraphicsPipeline& gl::GraphicsPipeline::operator=(GraphicsPipeline&& old) noexcept
{
	if (this == &old)
	{
		return *this;
	}

	m_id = std::exchange(old.m_id, 0);
	return *this;
}
//=============================================================================
gl::ComputePipeline::ComputePipeline(const ComputePipelineInfo& info)
{
	assert(info.shader);
	GLuint program = glCreateProgram();
	if (!program) return;

	glAttachShader(program, info.shader->Handle());

	std::string infoLog;
	if (!linkProgram(program, infoLog))
	{
		glDeleteProgram(program);
		Error("Failed to compile compute pipeline.\n" + infoLog);
		return;
	}

	if (!info.name.empty())
		glObjectLabel(GL_PROGRAM, program, static_cast<GLsizei>(info.name.length()), info.name.data());

	GLint workgroupSize[3];
	glGetProgramiv(program, GL_COMPUTE_WORK_GROUP_SIZE, workgroupSize);

	assert(workgroupSize[0] <= CurrentDeviceProperties.limits.maxComputeWorkGroupSize[0] &&
		workgroupSize[1] <= CurrentDeviceProperties.limits.maxComputeWorkGroupSize[1] &&
		workgroupSize[2] <= CurrentDeviceProperties.limits.maxComputeWorkGroupSize[2]);
	assert(workgroupSize[0] * workgroupSize[1] * workgroupSize[2] <= CurrentDeviceProperties.limits.maxComputeWorkGroupInvocations);

	auto owning = gl::detail::ComputePipelineInfoOwning{ .name = std::string(info.name) };
	owning.uniformBlocks = reflectProgram(program, GL_UNIFORM_BLOCK);
	owning.storageBlocks = reflectProgram(program, GL_SHADER_STORAGE_BLOCK);
	owning.samplersAndImages = reflectProgram(program, GL_UNIFORM);

	owning.workgroupSize.width = static_cast<uint32_t>(workgroupSize[0]);
	owning.workgroupSize.height = static_cast<uint32_t>(workgroupSize[1]);
	owning.workgroupSize.depth = static_cast<uint32_t>(workgroupSize[2]);

	gComputePipelines.insert({ program, std::make_shared<const gl::detail::ComputePipelineInfoOwning>(std::move(owning)) });
	m_id = program;

	Debug("Created Compute Program with handle " + std::to_string(m_id));
}
//=============================================================================
gl::ComputePipeline::~ComputePipeline()
{
	if (m_id != 0)
	{
		Debug("Destroyed Compute program with handle " + std::to_string(m_id));
		auto it = gComputePipelines.find(static_cast<GLuint>(m_id));
		if (it == gComputePipelines.end())
		{
			std::unreachable();
			return;
		}

		glDeleteProgram(static_cast<GLuint>(m_id));
		gComputePipelines.erase(it);
	}
}
//=============================================================================
gl::ComputePipeline::ComputePipeline(ComputePipeline&& old) noexcept
	: m_id(std::exchange(old.m_id, 0))
{
}
//=============================================================================
gl::ComputePipeline& gl::ComputePipeline::operator=(ComputePipeline&& old) noexcept
{
	if (this == &old)
	{
		return *this;
	}

	m_id = std::exchange(old.m_id, 0);
	return *this;
}
//=============================================================================
Extent3D gl::ComputePipeline::WorkgroupSize() const
{
	return detail::GetComputePipelineInternal(m_id)->workgroupSize;
}
//=============================================================================