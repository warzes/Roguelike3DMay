#include "stdafx.h"
#include "RsmTechnique.h"

static glm::uint pcg_hash(glm::uint seed)
{
	glm::uint state = seed * 747796405u + 2891336453u;
	glm::uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

// Used to advance the PCG state.
static glm::uint rand_pcg(glm::uint& rng_state)
{
	glm::uint state = rng_state;
	rng_state = rng_state * 747796405u + 2891336453u;
	glm::uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

// Advances the prng state and returns the corresponding random float.
static float rng(glm::uint& state)
{
	glm::uint x = rand_pcg(state);
	state = x;
	return float(x) * glm::uintBitsToFloat(0x2f800004u);
}

static std::string LoadFileWithInclude(std::string_view path)
{
	char error[256] = {};
	char* included = stb_include_string(io::LoadFile(path).data(), nullptr, (char*)"ExampleData/shaders/rsm", (char*)"", error);
	std::string includedStr = included;
	free(included);
	return includedStr;
}

static gl4::ComputePipeline CreateRsmIndirectPipeline()
{
	auto cs = gl4::Shader(gl4::PipelineStage::ComputeShader, io::LoadFile("ExampleData/shaders/rsm/Indirect.comp.glsl"));
	return gl4::ComputePipeline({ .shader = &cs });
}

static gl4::ComputePipeline CreateRsmIndirectFilteredPipeline()
{
	auto cs = gl4::Shader(gl4::PipelineStage::ComputeShader,
		io::LoadFile("ExampleData/shaders/rsm/IndirectDitheredFiltered.comp.glsl"));
	return gl4::ComputePipeline({ .shader = &cs });
}

static gl4::ComputePipeline CreateRsmReprojectPipeline()
{
	auto cs = gl4::Shader(gl4::PipelineStage::ComputeShader, LoadFileWithInclude("ExampleData/shaders/rsm/Reproject.comp.glsl"));
	return gl4::ComputePipeline({ .shader = &cs });
}

static gl4::ComputePipeline CreateBilateral5x5Pipeline()
{
	auto cs = gl4::Shader(gl4::PipelineStage::ComputeShader, LoadFileWithInclude("ExampleData/shaders/rsm/Bilateral5x5.comp.glsl"));
	return gl4::ComputePipeline({ .shader = &cs });
}

static gl4::ComputePipeline CreateModulatePipeline()
{
	auto cs = gl4::Shader(gl4::PipelineStage::ComputeShader, LoadFileWithInclude("ExampleData/shaders/rsm/Modulate.comp.glsl"));
	return gl4::ComputePipeline({ .shader = &cs });
}

static gl4::ComputePipeline CreateModulateUpscalePipeline()
{
	auto cs =
		gl4::Shader(gl4::PipelineStage::ComputeShader, LoadFileWithInclude("ExampleData/shaders/rsm/ModulateUpscale.comp.glsl"));
	return gl4::ComputePipeline({ .shader = &cs });
}

static gl4::ComputePipeline CreateBlitPipeline()
{
	auto cs = gl4::Shader(gl4::PipelineStage::ComputeShader, LoadFileWithInclude("ExampleData/shaders/rsm/BlitTexture.comp.glsl"));
	return gl4::ComputePipeline({ .shader = &cs });
}

namespace RSM
{
	RsmTechnique::RsmTechnique(uint32_t width_, uint32_t height_)
		: seedX(pcg_hash(17)),
		seedY(pcg_hash(seedX)),
		rsmUniformBuffer(gl4::BufferStorageFlag::DynamicStorage),
		cameraUniformBuffer(gl4::BufferStorageFlag::DynamicStorage),
		reprojectionUniformBuffer(gl4::BufferStorageFlag::DynamicStorage),
		filterUniformBuffer(gl4::BufferStorageFlag::DynamicStorage),
		rsmIndirectPipeline(CreateRsmIndirectPipeline()),
		rsmIndirectFilteredPipeline(CreateRsmIndirectFilteredPipeline()),
		rsmReprojectPipeline(CreateRsmReprojectPipeline()),
		bilateral5x5Pipeline(CreateBilateral5x5Pipeline()),
		modulatePipeline(CreateModulatePipeline()),
		modulateUpscalePipeline(CreateModulateUpscalePipeline()),
		blitPipeline(CreateBlitPipeline())
	{
		SetResolution(width_, height_);

		// load blue noise texture
		int x = 0;
		int y = 0;
		auto noise = std::unique_ptr < stbi_uc, decltype([](stbi_uc* p) { stbi_image_free(p); }) > (
			stbi_load("CoreData/textures/bluenoise256.png", &x, &y, nullptr, 4));

		assert(noise);
		noiseTex = gl4::CreateTexture2D({ static_cast<uint32_t>(x), static_cast<uint32_t>(y) }, gl4::Format::R8G8B8A8_UNORM);
		noiseTex->UpdateImage({
		.level = 0,
		.offset = {},
		.extent = {static_cast<uint32_t>(x), static_cast<uint32_t>(y)},
		.format = gl4::UploadFormat::RGBA,
		.type = gl4::UploadType::UBYTE,
		.pixels = noise.get(),
			});
	}

	void RsmTechnique::SetResolution(uint32_t newWidth, uint32_t newHeight)
	{
		width = newWidth;
		height = newHeight;
		internalWidth = width / inverseResolutionScale;
		internalHeight = height / inverseResolutionScale;
		indirectUnfilteredTex = gl4::CreateTexture2D({ internalWidth, internalHeight }, gl4::Format::R16G16B16A16_FLOAT);
		indirectUnfilteredTexPrev = gl4::CreateTexture2D({ internalWidth, internalHeight }, gl4::Format::R16G16B16A16_FLOAT);
		indirectFilteredTex = gl4::CreateTexture2D({ internalWidth, internalHeight }, gl4::Format::R16G16B16A16_FLOAT);
		indirectFilteredTexPingPong = gl4::CreateTexture2D({ internalWidth, internalHeight }, gl4::Format::R16G16B16A16_FLOAT);
		historyLengthTex = gl4::CreateTexture2D({ internalWidth, internalHeight }, gl4::Format::R8_UINT);
		illuminationUpscaled = gl4::CreateTexture2D({ width, height }, gl4::Format::R16G16B16A16_FLOAT);
		rsmFluxSmall = gl4::CreateTexture2D({ (uint32_t)smallRsmSize, (uint32_t)smallRsmSize }, gl4::Format::R11G11B10_FLOAT);
		rsmNormalSmall = gl4::CreateTexture2D({ (uint32_t)smallRsmSize, (uint32_t)smallRsmSize }, gl4::Format::R8G8B8A8_SNORM);
		rsmDepthSmall = gl4::CreateTexture2D({ (uint32_t)smallRsmSize, (uint32_t)smallRsmSize }, gl4::Format::R32_FLOAT);

		if (inverseResolutionScale > 1)
		{
			gNormalSmall = gl4::CreateTexture2D({ internalWidth, internalHeight }, gl4::Format::R8G8B8A8_SNORM);
			gNormalPrevSmall = gl4::CreateTexture2D({ internalWidth, internalHeight }, gl4::Format::R8G8B8A8_SNORM);
			gDepthSmall = gl4::CreateTexture2D({ internalWidth, internalHeight }, gl4::Format::R32_FLOAT);
			gDepthPrevSmall = gl4::CreateTexture2D({ internalWidth, internalHeight }, gl4::Format::R32_FLOAT);
		}
		else
		{
			gNormalSmall.reset();
			gNormalPrevSmall.reset();
			gDepthSmall.reset();
			gDepthPrevSmall.reset();
		}

		historyLengthTex->ClearImage({
		.extent = historyLengthTex->Extent(),
		.format = gl4::UploadFormat::R_INTEGER,
		.type = gl4::UploadType::UBYTE,
		.data = nullptr,
			});

		indirectUnfilteredTex->ClearImage({
		.extent = indirectUnfilteredTex->Extent(),
		.format = gl4::UploadFormat::RGBA,
		.type = gl4::UploadType::UBYTE,
		.data = nullptr,
			});
	}

	void RsmTechnique::ComputeIndirectLighting(const glm::mat4& lightViewProj,
		const CameraUniforms& cameraUniforms,
		const gl4::Texture& gAlbedo,
		const gl4::Texture& gNormal,
		const gl4::Texture& gDepth,
		const gl4::Texture& rsmFlux,
		const gl4::Texture& rsmNormal,
		const gl4::Texture& rsmDepth,
		const gl4::Texture& gDepthPrev,
		const gl4::Texture& gNormalPrev,
		const gl4::Texture& gMotion)
	{
		gl4::SamplerState ss;
		ss.minFilter = gl4::MinFilter::Nearest;
		ss.magFilter = gl4::MagFilter::Nearest;
		ss.addressModeU = gl4::AddressMode::Repeat;
		ss.addressModeV = gl4::AddressMode::Repeat;
		auto nearestSampler = gl4::Sampler(ss);

		ss.borderColor = gl4::BorderColor::FloatTransparentBlack;
		ss.addressModeU = gl4::AddressMode::ClampToBorder;
		ss.addressModeV = gl4::AddressMode::ClampToBorder;
		auto nearestSamplerClamped = gl4::Sampler(ss);

		ss.minFilter = gl4::MinFilter::Linear;
		ss.magFilter = gl4::MagFilter::Linear;
		auto linearSampler = gl4::Sampler(ss);

		rsmUniforms = {
		.sunViewProj = lightViewProj,
		.invSunViewProj = glm::inverse(lightViewProj),
		.rMax = rMax,
		.samples = static_cast<uint32_t>(rsmFiltered ? rsmFilteredSamples : rsmSamples),
		};

		if (seedEachFrame)
		{
			rsmUniforms.random = glm::vec2(rng(seedX), rng(seedY));
		}
		else
		{
			rsmUniforms.random = glm::vec2(0);
		}

		if (rsmFiltered)
		{
			rsmUniforms.targetDim = { internalWidth, internalHeight };
		}
		else
		{
			rsmUniforms.targetDim = { illuminationUpscaled->Extent().width, illuminationUpscaled->Extent().height };
		}

		rsmUniformBuffer.UpdateData(rsmUniforms);

		cameraUniformBuffer.UpdateData(cameraUniforms);

		gl4::BeginCompute("Indirect Illumination");
		{
			gl4::Cmd::BindSampledImage(0, *indirectUnfilteredTex, nearestSampler);
			gl4::Cmd::BindSampledImage(1, gAlbedo, nearestSampler);
			gl4::Cmd::BindSampledImage(2, gNormalSmall ? *gNormalSmall : gNormal, nearestSampler);
			gl4::Cmd::BindSampledImage(3, gDepthSmall ? *gDepthSmall : gDepth, nearestSampler);
			gl4::Cmd::BindSampledImage(4, *rsmFluxSmall, nearestSamplerClamped);
			gl4::Cmd::BindSampledImage(5, *rsmNormalSmall, nearestSampler);
			gl4::Cmd::BindSampledImage(6, *rsmDepthSmall, nearestSampler);
			gl4::Cmd::BindUniformBuffer(0, cameraUniformBuffer);
			gl4::Cmd::BindUniformBuffer(1, rsmUniformBuffer);

			if (rsmFiltered)
			{
				const auto workSize = Extent3D{ (uint32_t)rsmUniforms.targetDim.x, (uint32_t)rsmUniforms.targetDim.y, 1 };

				if (inverseResolutionScale > 1)
				{
					gl4::ScopedDebugMarker marker2("Downsample G-buffer");

					gl4::Cmd::BindComputePipeline(blitPipeline);
					gl4::Cmd::BindSampledImage(0, gNormal, nearestSampler);
					gl4::Cmd::BindImage(0, *gNormalSmall, 0);
					gl4::Cmd::DispatchInvocations(workSize);

					gl4::Cmd::BindSampledImage(0, gNormalPrev, nearestSampler);
					gl4::Cmd::BindImage(0, *gNormalPrevSmall, 0);
					gl4::Cmd::DispatchInvocations(workSize);

					gl4::Cmd::BindSampledImage(0, gDepth, nearestSampler);
					gl4::Cmd::BindImage(0, *gDepthSmall, 0);
					gl4::Cmd::DispatchInvocations(workSize);

					gl4::Cmd::BindSampledImage(0, gDepthPrev, nearestSampler);
					gl4::Cmd::BindImage(0, *gDepthPrevSmall, 0);
					gl4::Cmd::DispatchInvocations(workSize);
				}

				{
					gl4::ScopedDebugMarker marker2("Downsample RSM");

					gl4::Cmd::BindComputePipeline(blitPipeline);

					gl4::Cmd::BindSampledImage(0, rsmFlux, nearestSampler);
					gl4::Cmd::BindImage(0, *rsmFluxSmall, 0);
					gl4::Cmd::DispatchInvocations(rsmFluxSmall->Extent());

					gl4::Cmd::BindSampledImage(0, rsmNormal, nearestSampler);
					gl4::Cmd::BindImage(0, *rsmNormalSmall, 0);
					gl4::Cmd::DispatchInvocations(rsmNormalSmall->Extent());

					gl4::Cmd::BindSampledImage(0, rsmDepth, nearestSampler);
					gl4::Cmd::BindImage(0, *rsmDepthSmall, 0);
					gl4::Cmd::DispatchInvocations(rsmDepthSmall->Extent());
				}

				rsmUniforms.currentPass = 0;

				// Evaluate indirect illumination (sample the reflective shadow map)
				{
					gl4::ScopedDebugMarker marker2("Sample RSM");

					gl4::Cmd::BindComputePipeline(rsmIndirectFilteredPipeline);
					gl4::Cmd::BindSampledImage(7, *noiseTex, nearestSampler);
					rsmUniformBuffer.UpdateData(rsmUniforms);
					gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT | gl4::MemoryBarrierBit::IMAGE_ACCESS_BIT);
					gl4::Cmd::BindImage(0, *indirectUnfilteredTex, 0);
					gl4::Cmd::DispatchInvocations(workSize);
				}

				// Temporally accumulate samples before filtering
				{
					gl4::ScopedDebugMarker marker2("Temporal Accumulation");

					ReprojectionUniforms reprojectionUniforms = {
					.invViewProjCurrent = cameraUniforms.invViewProj,
					.viewProjPrevious = viewProjPrevious,
					.invViewProjPrevious = glm::inverse(viewProjPrevious),
					.proj = cameraUniforms.proj,
					.viewPos = cameraUniforms.cameraPos,
					.temporalWeightFactor = spatialFilterStep,
					.targetDim = {indirectUnfilteredTex->Extent().width, indirectUnfilteredTex->Extent().height},
					.alphaIlluminance = alphaIlluminance,
					.phiDepth = phiDepth,
					.phiNormal = phiNormal,
					.jitterOffset = cameraUniforms.jitterOffset,
					.lastFrameJitterOffset = cameraUniforms.lastFrameJitterOffset,
					};
					viewProjPrevious = cameraUniforms.viewProj;
					reprojectionUniformBuffer.UpdateData(reprojectionUniforms);
					gl4::Cmd::BindComputePipeline(rsmReprojectPipeline);
					gl4::Cmd::BindSampledImage(0, *indirectUnfilteredTex, nearestSampler);
					gl4::Cmd::BindSampledImage(1, *indirectUnfilteredTexPrev, linearSampler);
					gl4::Cmd::BindSampledImage(2, gDepthSmall ? *gDepthSmall : gDepth, nearestSampler);
					gl4::Cmd::BindSampledImage(3, gDepthPrevSmall ? *gDepthPrevSmall : gDepthPrev, linearSampler);
					gl4::Cmd::BindSampledImage(4, gNormalSmall ? *gNormalSmall : gNormal, nearestSampler);
					gl4::Cmd::BindSampledImage(5, gNormalPrevSmall ? *gNormalPrevSmall : gNormalPrev, linearSampler);
					gl4::Cmd::BindSampledImage(6, gMotion, linearSampler);
					gl4::Cmd::BindImage(0, *indirectFilteredTex, 0);
					gl4::Cmd::BindImage(1, *historyLengthTex, 0);
					gl4::Cmd::BindUniformBuffer(0, reprojectionUniformBuffer);
					gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT | gl4::MemoryBarrierBit::IMAGE_ACCESS_BIT);
					gl4::Cmd::DispatchInvocations(workSize);
				}

				FilterUniforms filterUniforms = {
				.proj = cameraUniforms.proj,
				.invViewProj = cameraUniforms.invViewProj,
				.viewPos = cameraUniforms.cameraPos,
				.targetDim = {indirectUnfilteredTex->Extent().width, indirectFilteredTex->Extent().height},
				.phiNormal = phiNormal,
				.phiDepth = phiDepth,
				};

				{
					gl4::ScopedDebugMarker marker2("Filter");

					gl4::Cmd::BindComputePipeline(bilateral5x5Pipeline);
					gl4::Cmd::BindSampledImage(1, gNormalSmall ? *gNormalSmall : gNormal, nearestSampler);
					gl4::Cmd::BindSampledImage(2, gDepthSmall ? *gDepthSmall : gDepth, nearestSampler);
					gl4::Cmd::BindSampledImage(3, *historyLengthTex, nearestSampler);
					gl4::Cmd::BindUniformBuffer(0, filterUniformBuffer);

					if (useSeparableFilter)
					{
						filterUniforms.stepWidth = 1 * spatialFilterStep;

						filterUniforms.direction = { 0, 1 };
						filterUniformBuffer.UpdateData(filterUniforms);
						gl4::Cmd::BindSampledImage(0, *indirectFilteredTex, nearestSampler);
						gl4::Cmd::BindImage(0, *indirectUnfilteredTex, 0);
						gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT);
						gl4::Cmd::DispatchInvocations(workSize);

						filterUniforms.direction = { 1, 0 };
						filterUniformBuffer.UpdateData(filterUniforms);
						gl4::Cmd::BindSampledImage(0, *indirectUnfilteredTex, nearestSampler);
						gl4::Cmd::BindImage(0, *indirectUnfilteredTexPrev, 0);
						gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT);
						gl4::Cmd::DispatchInvocations(workSize);

						for (int i = 1; i < 5 - std::log2f(float(inverseResolutionScale)); i++)
						{
							filterUniforms.stepWidth = (1 << i) * spatialFilterStep;

							filterUniforms.direction = { 0, 1 };
							filterUniformBuffer.UpdateData(filterUniforms);
							gl4::Cmd::BindSampledImage(0, i == 1 ? *indirectUnfilteredTexPrev : *indirectFilteredTex, nearestSampler);
							gl4::Cmd::BindImage(0, *indirectUnfilteredTex, 0);
							gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT);
							gl4::Cmd::DispatchInvocations(workSize);

							filterUniforms.direction = { 1, 0 };
							filterUniformBuffer.UpdateData(filterUniforms);
							gl4::Cmd::BindSampledImage(0, *indirectUnfilteredTex, nearestSampler);
							gl4::Cmd::BindImage(0, *indirectFilteredTex, 0);
							gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT);
							gl4::Cmd::DispatchInvocations(workSize);
						}
					}
					else
					{
						filterUniforms.direction = { 0, 0 };

						for (int i = 0; i < 5 - std::log2f(float(inverseResolutionScale)); i++)
						{
							filterUniforms.stepWidth = (1 << i) * spatialFilterStep;
							filterUniformBuffer.UpdateData(filterUniforms);

							// The output of the first filter pass gets stored in the history
							const gl4::Texture* in{};
							const gl4::Texture* out{};
							if (i == 0)
							{
								in = &indirectFilteredTex.value();
								out = &indirectUnfilteredTexPrev.value();
							}
							else if (i == 1)
							{
								in = &indirectUnfilteredTexPrev.value();
								out = &indirectUnfilteredTex.value();
							}
							else if (i % 2 == 0)
							{
								in = &indirectUnfilteredTex.value();
								out = &indirectFilteredTex.value();
							}
							else
							{
								in = &indirectFilteredTex.value();
								out = &indirectUnfilteredTex.value();
							}

							gl4::Cmd::BindSampledImage(0, *in, nearestSampler);
							gl4::Cmd::BindImage(0, *out, 0);
							gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT);
							gl4::Cmd::DispatchInvocations(workSize);
						}
					}
				}

				auto& illuminationOutTex = inverseResolutionScale == 1 ? indirectFilteredTexPingPong : illuminationUpscaled;
				if (!rsmFilteredSkipAlbedoModulation)
				{
					// No upscale required
					if (inverseResolutionScale == 1)
					{
						gl4::ScopedDebugMarker marker2("Modulate Albedo");

						gl4::Cmd::BindComputePipeline(modulatePipeline);
						gl4::Cmd::BindSampledImage(0, *indirectFilteredTex, nearestSampler);
						gl4::Cmd::BindSampledImage(1, gAlbedo, nearestSampler);
						gl4::Cmd::BindImage(0, *illuminationOutTex, 0);
						gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT);
						gl4::Cmd::DispatchInvocations(illuminationOutTex->Extent());
					}
					else // Use bilateral upscale
					{
						gl4::ScopedDebugMarker marker2("Modulate Albedo (Upscale)");

						gl4::Cmd::BindComputePipeline(modulateUpscalePipeline);

						if (!useSeparableFilter && (5 - int(std::log2f(float(inverseResolutionScale)))) % 2 == 0)
						{
							gl4::Cmd::BindSampledImage(0, *indirectUnfilteredTex, nearestSampler);
						}
						else
						{
							gl4::Cmd::BindSampledImage(0, *indirectFilteredTex, nearestSampler);
						}

						gl4::Cmd::BindSampledImage(1, gAlbedo, nearestSampler);
						gl4::Cmd::BindSampledImage(2, gNormal, nearestSampler);
						gl4::Cmd::BindSampledImage(3, gDepth, nearestSampler);
						gl4::Cmd::BindSampledImage(4, *gNormalSmall, nearestSampler);
						gl4::Cmd::BindSampledImage(5, *gDepthSmall, nearestSampler);
						filterUniforms.targetDim = { illuminationOutTex->Extent().width, illuminationOutTex->Extent().height };
						filterUniformBuffer.UpdateData(filterUniforms);
						gl4::Cmd::BindUniformBuffer(0, filterUniformBuffer);
						gl4::Cmd::BindImage(0, *illuminationOutTex, 0);
						gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT);
						gl4::Cmd::DispatchInvocations(illuminationOutTex->Extent());
					}
				}
				else
				{
					gl4::BlitTexture(*indirectFilteredTex,
						*illuminationOutTex,
						{},
						{},
						indirectFilteredTex->Extent(),
						illuminationOutTex->Extent(),
						gl4::MagFilter::Nearest);
				}
			}
			else // Unfiltered RSM: the original paper
			{
				gl4::Cmd::BindComputePipeline(rsmIndirectPipeline);
				gl4::Cmd::BindSampledImage(1, gAlbedo, nearestSampler);
				gl4::Cmd::BindSampledImage(2, gNormal, nearestSampler);
				gl4::Cmd::BindSampledImage(3, gDepth, nearestSampler);
				gl4::Cmd::BindSampledImage(4, rsmFlux, nearestSamplerClamped);
				gl4::Cmd::BindSampledImage(5, rsmNormal, nearestSampler);
				gl4::Cmd::BindSampledImage(6, rsmDepth, nearestSampler);
				gl4::Cmd::BindSampledImage(0, *illuminationUpscaled, nearestSampler);
				gl4::Cmd::BindImage(0, *illuminationUpscaled, 0);

				const auto workgroupSize = rsmIndirectFilteredPipeline.WorkgroupSize();
				const auto workSize =
					Extent3D{ (uint32_t)rsmUniforms.targetDim.x / 2, (uint32_t)rsmUniforms.targetDim.y / 2, 1 };

				// Quarter resolution indirect illumination pass
				rsmUniforms.currentPass = 0;
				rsmUniformBuffer.UpdateData(rsmUniforms);
				gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT);
				gl4::Cmd::DispatchInvocations(workSize);

				// Reconstruction pass 1
				rsmUniforms.currentPass = 1;
				rsmUniformBuffer.UpdateData(rsmUniforms);
				gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT);
				gl4::Cmd::DispatchInvocations(workSize);

				// Reconstruction pass 2
				rsmUniforms.currentPass = 2;
				rsmUniformBuffer.UpdateData(rsmUniforms);
				gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT);
				gl4::Cmd::DispatchInvocations(workSize);

				// Reconstruction pass 3
				rsmUniforms.currentPass = 3;
				rsmUniformBuffer.UpdateData(rsmUniforms);
				gl4::MemoryBarrier(gl4::MemoryBarrierBit::TEXTURE_FETCH_BIT);
				gl4::Cmd::DispatchInvocations(workSize);
			}
		}
		gl4::EndCompute();
	}

	gl4::Texture& RsmTechnique::GetIndirectLighting()
	{
		if (rsmFiltered)
		{
			return inverseResolutionScale == 1 ? *indirectFilteredTexPingPong : *illuminationUpscaled;
		}

		return *illuminationUpscaled;
	}

	void RsmTechnique::DrawGui()
	{
		ImGui::Checkbox("Use Filtered RSM", &rsmFiltered);

		// This setting blows everything up, so it'll be hardcoded for now
		// if (ImGui::SliderInt("Inverse Res. Scale", &inverseResolutionScale, 1, 4))
		//{
		//  SetResolution(width, height);
		//}
		if (ImGui::SliderInt("Small RSM Size", &smallRsmSize, 64, 1024))
		{
			SetResolution(width, height);
		}
		ImGui::SliderFloat("rMax", &rMax, 0.02f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
		ImGui::PushButtonRepeat(true);

		if (!rsmFiltered)
		{
			ImGui::SliderInt("Samples##Uniltered", &rsmSamples, 1, 400);
			ImGui::SameLine();
			if (ImGui::Button(" - "))
				rsmSamples--;
			ImGui::SameLine();
			if (ImGui::Button(" + "))
				rsmSamples++;
			rsmSamples = rsmSamples <= 0 ? 1 : rsmSamples;
		}
		else
		{
			ImGui::SliderInt("Samples##Filtered", &rsmFilteredSamples, 1, 40);
			ImGui::SameLine();
			if (ImGui::Button(" - "))
				rsmFilteredSamples--;
			ImGui::SameLine();
			if (ImGui::Button(" + "))
				rsmFilteredSamples++;
			rsmFilteredSamples = rsmFilteredSamples <= 0 ? 1 : rsmFilteredSamples;

			float epsilon = 1e-2f;
			ImGui::SliderFloat("Alpha Illuminance", &alphaIlluminance, epsilon, 1);
			ImGui::SliderFloat("Phi Normal", &phiNormal, epsilon, 1);
			ImGui::SliderFloat("Phi Depth", &phiDepth, epsilon, 1);
			ImGui::SliderFloat("Spatial Filter Step", &spatialFilterStep, 0, 1);
			ImGui::Checkbox("Skip Albedo Modulation", &rsmFilteredSkipAlbedoModulation);
			ImGui::Checkbox("Seed Each Frame", &seedEachFrame);
			ImGui::Checkbox("Use Separable Filter", &useSeparableFilter);
		}

		rsmUniforms.samples = static_cast<uint32_t>(rsmFiltered ? rsmFilteredSamples : rsmSamples);

		ImGui::PopButtonRepeat();
	}
} // namespace RSM