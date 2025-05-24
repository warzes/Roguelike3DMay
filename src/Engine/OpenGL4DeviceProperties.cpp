﻿#include "stdafx.h"
#include "OpenGL4DeviceProperties.h"
//=============================================================================
void gl4::InitDeviceProperties()
{
	gDeviceProperties.vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
	gDeviceProperties.renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	gDeviceProperties.version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	gDeviceProperties.shadingLanguageVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

	glGetIntegerv(GL_MAJOR_VERSION, &gDeviceProperties.glVersionMajor);
	glGetIntegerv(GL_MINOR_VERSION, &gDeviceProperties.glVersionMinor);

	// Populate limits
	auto& limits = gDeviceProperties.limits;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &limits.maxTextureSize);
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &limits.maxTextureSize3D);
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &limits.maxTextureSizeCube);

	glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &limits.maxSamplerLodBias);
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &limits.maxSamplerAnisotropy);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &limits.maxArrayTextureLayers);
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, limits.maxViewportDims);
	glGetIntegerv(GL_SUBPIXEL_BITS, &limits.subpixelBits);

	glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &limits.maxFramebufferWidth);
	glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &limits.maxFramebufferHeight);
	glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS, &limits.maxFramebufferLayers);
	glGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &limits.maxFramebufferSamples);
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &limits.maxColorAttachments);
	glGetIntegerv(GL_MAX_SAMPLES, &limits.maxSamples);
	glGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &limits.maxSamplesNoAttachments);

	glGetFloatv(GL_MIN_FRAGMENT_INTERPOLATION_OFFSET, &limits.interpolationOffsetRange[0]);
	glGetFloatv(GL_MAX_FRAGMENT_INTERPOLATION_OFFSET, &limits.interpolationOffsetRange[1]);
	glGetFloatv(GL_POINT_SIZE_GRANULARITY, &limits.pointSizeGranularity);
	glGetFloatv(GL_POINT_SIZE_RANGE, limits.pointSizeRange);
	glGetFloatv(GL_LINE_WIDTH_RANGE, limits.lineWidthRange);

	glGetIntegerv(GL_MAX_ELEMENT_INDEX, &limits.maxElementIndex);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &limits.maxVertexAttribs);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &limits.maxVertexAttribBindings);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIB_STRIDE, &limits.maxVertexAttribStride);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET, &limits.maxVertexAttribRelativeOffset);
	glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &limits.maxVertexOutputComponents);
	glGetIntegerv(GL_MAX_TESS_CONTROL_INPUT_COMPONENTS, &limits.maxTessellationControlPerVertexInputComponents);
	glGetIntegerv(GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS, &limits.maxTessellationControlPerVertexOutputComponents);
	glGetIntegerv(GL_MAX_TESS_PATCH_COMPONENTS, &limits.maxTessellationControlPerPatchOutputComponents);
	glGetIntegerv(GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS, &limits.maxTessellationControlTotalOutputComponents);
	glGetIntegerv(GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS, &limits.maxTessellationEvaluationInputComponents);
	glGetIntegerv(GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS, &limits.maxTessellationEvaluationOutputComponents);
	glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &limits.maxFragmentInputComponents);
	glGetIntegerv(GL_MIN_PROGRAM_TEXEL_OFFSET, &limits.texelOffsetRange[0]);
	glGetIntegerv(GL_MAX_PROGRAM_TEXEL_OFFSET, &limits.texelOffsetRange[1]);
	glGetIntegerv(GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET, &limits.textureGatherOffsetRange[0]);
	glGetIntegerv(GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET, &limits.textureGatherOffsetRange[1]);

	glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &limits.maxTessellationGenerationLevel);
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &limits.maxPatchSize);

	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &limits.maxUniformBufferBindings);
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &limits.maxUniformBlockSize);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &limits.uniformBufferOffsetAlignment);
	glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS, &limits.maxCombinedUniformBlocks);

	glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &limits.maxShaderStorageBufferBindings);
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &limits.maxShaderStorageBlockSize);
	glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &limits.shaderStorageBufferOffsetAlignment);
	glGetIntegerv(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, &limits.maxCombinedShaderStorageBlocks);

	glGetIntegerv(GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES, &limits.maxCombinedShaderOutputResources);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &limits.maxCombinedTextureImageUnits);

	glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &limits.maxComputeSharedMemorySize);
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &limits.maxComputeWorkGroupInvocations);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &limits.maxComputeWorkGroupCount[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &limits.maxComputeWorkGroupCount[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &limits.maxComputeWorkGroupCount[2]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &limits.maxComputeWorkGroupSize[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &limits.maxComputeWorkGroupSize[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &limits.maxComputeWorkGroupSize[2]);

	glGetIntegerv(GL_MAX_IMAGE_UNITS, &limits.maxImageUnits);
	glGetIntegerv(GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS, &limits.maxFragmentCombinedOutputResources);
	glGetIntegerv(GL_MAX_COMBINED_IMAGE_UNIFORMS, &limits.maxCombinedImageUniforms);
	glGetIntegerv(GL_MAX_SERVER_WAIT_TIMEOUT, &limits.maxServerWaitTimeout);

	// Populate features
	auto& features = gDeviceProperties.features;

	GLint numExtensions{};
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	for (GLuint i = 0; i < static_cast<GLuint>(numExtensions); i++)
	{
		std::string_view extensionString = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
		// printf("%s\n", extensionString.data());
		if (extensionString == "GL_ARB_bindless_texture")
		{
			features.bindlessTextures = true;
		}

		if (extensionString == "GL_KHR_shader_subgroup")
		{
			features.shaderSubgroup = true;

			glGetIntegerv(GL_SUBGROUP_SIZE_KHR, &limits.subgroupLimits.subgroupSize);

			int32_t subgroupStages{};
			glGetIntegerv(GL_SUBGROUP_SUPPORTED_STAGES_KHR, &subgroupStages);
			limits.subgroupLimits.vertexShaderSupported = subgroupStages & GL_VERTEX_SHADER_BIT;
			limits.subgroupLimits.tessellationControlShaderSupported = subgroupStages & GL_TESS_CONTROL_SHADER_BIT;
			limits.subgroupLimits.tessellationEvaluationShaderSupported = subgroupStages & GL_TESS_EVALUATION_SHADER_BIT;
			limits.subgroupLimits.fragmentShaderSupported = subgroupStages & GL_FRAGMENT_SHADER_BIT;
			limits.subgroupLimits.computeShaderSupported = subgroupStages & GL_COMPUTE_SHADER_BIT;

			int32_t subgroupFeatures{};
			glGetIntegerv(GL_SUBGROUP_SUPPORTED_FEATURES_KHR, &subgroupFeatures);
			limits.subgroupLimits.voteSupported = subgroupFeatures & GL_SUBGROUP_FEATURE_VOTE_BIT_KHR;
			limits.subgroupLimits.arithmeticSupported = subgroupFeatures & GL_SUBGROUP_FEATURE_ARITHMETIC_BIT_KHR;
			limits.subgroupLimits.ballotSupported = subgroupFeatures & GL_SUBGROUP_FEATURE_BALLOT_BIT_KHR;
			limits.subgroupLimits.shuffleSupported = subgroupFeatures & GL_SUBGROUP_FEATURE_SHUFFLE_BIT_KHR;
			limits.subgroupLimits.shuffleRelativeSupported = subgroupFeatures & GL_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT_KHR;
			limits.subgroupLimits.clusteredSupported = subgroupFeatures & GL_SUBGROUP_FEATURE_CLUSTERED_BIT_KHR;
			limits.subgroupLimits.quadSupported = subgroupFeatures & GL_SUBGROUP_FEATURE_QUAD_BIT_KHR;
		}
	}
}
//=============================================================================