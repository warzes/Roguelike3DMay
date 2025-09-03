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

static gl::ComputePipeline CreateRsmIndirectPipeline()
{
	auto cs = gl::Shader(gl::ShaderType::ComputeShader, io::LoadFile("ExampleData/shaders/rsm/Indirect.comp.glsl"));
	return gl::ComputePipeline({ .shader = &cs });
}

static gl::ComputePipeline CreateRsmIndirectFilteredPipeline()
{
	auto cs = gl::Shader(gl::ShaderType::ComputeShader,
		io::LoadFile("ExampleData/shaders/rsm/IndirectDitheredFiltered.comp.glsl"));
	return gl::ComputePipeline({ .shader = &cs });
}

static gl::ComputePipeline CreateRsmReprojectPipeline()
{
	auto cs = gl::Shader(gl::ShaderType::ComputeShader, LoadFileWithInclude("ExampleData/shaders/rsm/Reproject.comp.glsl"));
	return gl::ComputePipeline({ .shader = &cs });
}

static gl::ComputePipeline CreateBilateral5x5Pipeline()
{
	auto cs = gl::Shader(gl::ShaderType::ComputeShader, LoadFileWithInclude("ExampleData/shaders/rsm/Bilateral5x5.comp.glsl"));
	return gl::ComputePipeline({ .shader = &cs });
}

static gl::ComputePipeline CreateModulatePipeline()
{
	auto cs = gl::Shader(gl::ShaderType::ComputeShader, LoadFileWithInclude("ExampleData/shaders/rsm/Modulate.comp.glsl"));
	return gl::ComputePipeline({ .shader = &cs });
}

static gl::ComputePipeline CreateModulateUpscalePipeline()
{
	auto cs =
		gl::Shader(gl::ShaderType::ComputeShader, LoadFileWithInclude("ExampleData/shaders/rsm/ModulateUpscale.comp.glsl"));
	return gl::ComputePipeline({ .shader = &cs });
}

static gl::ComputePipeline CreateBlitPipeline()
{
	auto cs = gl::Shader(gl::ShaderType::ComputeShader, LoadFileWithInclude("ExampleData/shaders/rsm/BlitTexture.comp.glsl"));
	return gl::ComputePipeline({ .shader = &cs });
}

namespace RSM
{
	RsmTechnique::RsmTechnique(uint32_t width_, uint32_t height_)
		: seedX(pcg_hash(17)),
		seedY(pcg_hash(seedX)),
		rsmUniformBuffer(gl::BufferStorageFlag::DynamicStorage),
		cameraUniformBuffer(gl::BufferStorageFlag::DynamicStorage),
		reprojectionUniformBuffer(gl::BufferStorageFlag::DynamicStorage),
		filterUniformBuffer(gl::BufferStorageFlag::DynamicStorage),
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
		noiseTex = gl::CreateTexture2D({ static_cast<uint32_t>(x), static_cast<uint32_t>(y) }, gl::Format::R8G8B8A8_UNORM);
		noiseTex->UpdateImage({
		.level = 0,
		.offset = {},
		.extent = {static_cast<uint32_t>(x), static_cast<uint32_t>(y)},
		.format = gl::UploadFormat::RGBA,
		.type = gl::UploadType::UBYTE,
		.pixels = noise.get(),
			});
	}

	void RsmTechnique::SetResolution(uint32_t newWidth, uint32_t newHeight)
	{
		width = newWidth;
		height = newHeight;
		internalWidth = width / inverseResolutionScale;
		internalHeight = height / inverseResolutionScale;
		indirectUnfilteredTex = gl::CreateTexture2D({ internalWidth, internalHeight }, gl::Format::R16G16B16A16_FLOAT);
		indirectUnfilteredTexPrev = gl::CreateTexture2D({ internalWidth, internalHeight }, gl::Format::R16G16B16A16_FLOAT);
		indirectFilteredTex = gl::CreateTexture2D({ internalWidth, internalHeight }, gl::Format::R16G16B16A16_FLOAT);
		indirectFilteredTexPingPong = gl::CreateTexture2D({ internalWidth, internalHeight }, gl::Format::R16G16B16A16_FLOAT);
		historyLengthTex = gl::CreateTexture2D({ internalWidth, internalHeight }, gl::Format::R8_UINT);
		illuminationUpscaled = gl::CreateTexture2D({ width, height }, gl::Format::R16G16B16A16_FLOAT);
		rsmFluxSmall = gl::CreateTexture2D({ (uint32_t)smallRsmSize, (uint32_t)smallRsmSize }, gl::Format::R11G11B10_FLOAT);
		rsmNormalSmall = gl::CreateTexture2D({ (uint32_t)smallRsmSize, (uint32_t)smallRsmSize }, gl::Format::R8G8B8A8_SNORM);
		rsmDepthSmall = gl::CreateTexture2D({ (uint32_t)smallRsmSize, (uint32_t)smallRsmSize }, gl::Format::R32_FLOAT);

		if (inverseResolutionScale > 1)
		{
			gNormalSmall = gl::CreateTexture2D({ internalWidth, internalHeight }, gl::Format::R8G8B8A8_SNORM);
			gNormalPrevSmall = gl::CreateTexture2D({ internalWidth, internalHeight }, gl::Format::R8G8B8A8_SNORM);
			gDepthSmall = gl::CreateTexture2D({ internalWidth, internalHeight }, gl::Format::R32_FLOAT);
			gDepthPrevSmall = gl::CreateTexture2D({ internalWidth, internalHeight }, gl::Format::R32_FLOAT);
		}
		else
		{
			gNormalSmall.reset();
			gNormalPrevSmall.reset();
			gDepthSmall.reset();
			gDepthPrevSmall.reset();
		}

		historyLengthTex->ClearImage({
		.extent = historyLengthTex->GetExtent(),
		.format = gl::UploadFormat::R_INTEGER,
		.type = gl::UploadType::UBYTE,
		.data = nullptr,
			});

		indirectUnfilteredTex->ClearImage({
		.extent = indirectUnfilteredTex->GetExtent(),
		.format = gl::UploadFormat::RGBA,
		.type = gl::UploadType::UBYTE,
		.data = nullptr,
			});
	}

	void RsmTechnique::ComputeIndirectLighting(const glm::mat4& lightViewProj,
		const CameraUniforms& cameraUniforms,
		const gl::Texture& gAlbedo,
		const gl::Texture& gNormal,
		const gl::Texture& gDepth,
		const gl::Texture& rsmFlux,
		const gl::Texture& rsmNormal,
		const gl::Texture& rsmDepth,
		const gl::Texture& gDepthPrev,
		const gl::Texture& gNormalPrev,
		const gl::Texture& gMotion)
	{
		gl::SamplerState ss;
		ss.minFilter = gl::MinFilter::Nearest;
		ss.magFilter = gl::MagFilter::Nearest;
		ss.addressModeU = gl::AddressMode::Repeat;
		ss.addressModeV = gl::AddressMode::Repeat;
		auto nearestSampler = gl::Sampler(ss);

		ss.borderColor = gl::BorderColor::FloatTransparentBlack;
		ss.addressModeU = gl::AddressMode::ClampToBorder;
		ss.addressModeV = gl::AddressMode::ClampToBorder;
		auto nearestSamplerClamped = gl::Sampler(ss);

		ss.minFilter = gl::MinFilter::Linear;
		ss.magFilter = gl::MagFilter::Linear;
		auto linearSampler = gl::Sampler(ss);

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
			rsmUniforms.targetDim = { illuminationUpscaled->GetExtent().width, illuminationUpscaled->GetExtent().height };
		}

		rsmUniformBuffer.UpdateData(rsmUniforms);

		cameraUniformBuffer.UpdateData(cameraUniforms);

		gl::BeginCompute("Indirect Illumination");
		{
			gl::Cmd::BindSampledImage(0, *indirectUnfilteredTex, nearestSampler);
			gl::Cmd::BindSampledImage(1, gAlbedo, nearestSampler);
			gl::Cmd::BindSampledImage(2, gNormalSmall ? *gNormalSmall : gNormal, nearestSampler);
			gl::Cmd::BindSampledImage(3, gDepthSmall ? *gDepthSmall : gDepth, nearestSampler);
			gl::Cmd::BindSampledImage(4, *rsmFluxSmall, nearestSamplerClamped);
			gl::Cmd::BindSampledImage(5, *rsmNormalSmall, nearestSampler);
			gl::Cmd::BindSampledImage(6, *rsmDepthSmall, nearestSampler);
			gl::Cmd::BindUniformBuffer(0, cameraUniformBuffer);
			gl::Cmd::BindUniformBuffer(1, rsmUniformBuffer);

			if (rsmFiltered)
			{
				const auto workSize = Extent3D{ (uint32_t)rsmUniforms.targetDim.x, (uint32_t)rsmUniforms.targetDim.y, 1 };

				if (inverseResolutionScale > 1)
				{
					gl::ScopedDebugMarker marker2("Downsample G-buffer");

					gl::Cmd::BindComputePipeline(blitPipeline);
					gl::Cmd::BindSampledImage(0, gNormal, nearestSampler);
					gl::Cmd::BindImage(0, *gNormalSmall, 0);
					gl::Cmd::DispatchInvocations(workSize);

					gl::Cmd::BindSampledImage(0, gNormalPrev, nearestSampler);
					gl::Cmd::BindImage(0, *gNormalPrevSmall, 0);
					gl::Cmd::DispatchInvocations(workSize);

					gl::Cmd::BindSampledImage(0, gDepth, nearestSampler);
					gl::Cmd::BindImage(0, *gDepthSmall, 0);
					gl::Cmd::DispatchInvocations(workSize);

					gl::Cmd::BindSampledImage(0, gDepthPrev, nearestSampler);
					gl::Cmd::BindImage(0, *gDepthPrevSmall, 0);
					gl::Cmd::DispatchInvocations(workSize);
				}

				{
					gl::ScopedDebugMarker marker2("Downsample RSM");

					gl::Cmd::BindComputePipeline(blitPipeline);

					gl::Cmd::BindSampledImage(0, rsmFlux, nearestSampler);
					gl::Cmd::BindImage(0, *rsmFluxSmall, 0);
					gl::Cmd::DispatchInvocations(rsmFluxSmall->GetExtent());

					gl::Cmd::BindSampledImage(0, rsmNormal, nearestSampler);
					gl::Cmd::BindImage(0, *rsmNormalSmall, 0);
					gl::Cmd::DispatchInvocations(rsmNormalSmall->GetExtent());

					gl::Cmd::BindSampledImage(0, rsmDepth, nearestSampler);
					gl::Cmd::BindImage(0, *rsmDepthSmall, 0);
					gl::Cmd::DispatchInvocations(rsmDepthSmall->GetExtent());
				}

				rsmUniforms.currentPass = 0;

				// Evaluate indirect illumination (sample the reflective shadow map)
				{
					gl::ScopedDebugMarker marker2("Sample RSM");

					gl::Cmd::BindComputePipeline(rsmIndirectFilteredPipeline);
					gl::Cmd::BindSampledImage(7, *noiseTex, nearestSampler);
					rsmUniformBuffer.UpdateData(rsmUniforms);
					gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit | gl::MemoryBarrierBit::ImageAccessBit);
					gl::Cmd::BindImage(0, *indirectUnfilteredTex, 0);
					gl::Cmd::DispatchInvocations(workSize);
				}

				// Temporally accumulate samples before filtering
				{
					gl::ScopedDebugMarker marker2("Temporal Accumulation");

					ReprojectionUniforms reprojectionUniforms = {
					.invViewProjCurrent = cameraUniforms.invViewProj,
					.viewProjPrevious = viewProjPrevious,
					.invViewProjPrevious = glm::inverse(viewProjPrevious),
					.proj = cameraUniforms.proj,
					.viewPos = cameraUniforms.cameraPos,
					.temporalWeightFactor = spatialFilterStep,
					.targetDim = {indirectUnfilteredTex->GetExtent().width, indirectUnfilteredTex->GetExtent().height},
					.alphaIlluminance = alphaIlluminance,
					.phiDepth = phiDepth,
					.phiNormal = phiNormal,
					.jitterOffset = cameraUniforms.jitterOffset,
					.lastFrameJitterOffset = cameraUniforms.lastFrameJitterOffset,
					};
					viewProjPrevious = cameraUniforms.viewProj;
					reprojectionUniformBuffer.UpdateData(reprojectionUniforms);
					gl::Cmd::BindComputePipeline(rsmReprojectPipeline);
					gl::Cmd::BindSampledImage(0, *indirectUnfilteredTex, nearestSampler);
					gl::Cmd::BindSampledImage(1, *indirectUnfilteredTexPrev, linearSampler);
					gl::Cmd::BindSampledImage(2, gDepthSmall ? *gDepthSmall : gDepth, nearestSampler);
					gl::Cmd::BindSampledImage(3, gDepthPrevSmall ? *gDepthPrevSmall : gDepthPrev, linearSampler);
					gl::Cmd::BindSampledImage(4, gNormalSmall ? *gNormalSmall : gNormal, nearestSampler);
					gl::Cmd::BindSampledImage(5, gNormalPrevSmall ? *gNormalPrevSmall : gNormalPrev, linearSampler);
					gl::Cmd::BindSampledImage(6, gMotion, linearSampler);
					gl::Cmd::BindImage(0, *indirectFilteredTex, 0);
					gl::Cmd::BindImage(1, *historyLengthTex, 0);
					gl::Cmd::BindUniformBuffer(0, reprojectionUniformBuffer);
					gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit | gl::MemoryBarrierBit::ImageAccessBit);
					gl::Cmd::DispatchInvocations(workSize);
				}

				FilterUniforms filterUniforms = {
				.proj = cameraUniforms.proj,
				.invViewProj = cameraUniforms.invViewProj,
				.viewPos = cameraUniforms.cameraPos,
				.targetDim = {indirectUnfilteredTex->GetExtent().width, indirectFilteredTex->GetExtent().height},
				.phiNormal = phiNormal,
				.phiDepth = phiDepth,
				};

				{
					gl::ScopedDebugMarker marker2("Filter");

					gl::Cmd::BindComputePipeline(bilateral5x5Pipeline);
					gl::Cmd::BindSampledImage(1, gNormalSmall ? *gNormalSmall : gNormal, nearestSampler);
					gl::Cmd::BindSampledImage(2, gDepthSmall ? *gDepthSmall : gDepth, nearestSampler);
					gl::Cmd::BindSampledImage(3, *historyLengthTex, nearestSampler);
					gl::Cmd::BindUniformBuffer(0, filterUniformBuffer);

					if (useSeparableFilter)
					{
						filterUniforms.stepWidth = 1 * spatialFilterStep;

						filterUniforms.direction = { 0, 1 };
						filterUniformBuffer.UpdateData(filterUniforms);
						gl::Cmd::BindSampledImage(0, *indirectFilteredTex, nearestSampler);
						gl::Cmd::BindImage(0, *indirectUnfilteredTex, 0);
						gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit);
						gl::Cmd::DispatchInvocations(workSize);

						filterUniforms.direction = { 1, 0 };
						filterUniformBuffer.UpdateData(filterUniforms);
						gl::Cmd::BindSampledImage(0, *indirectUnfilteredTex, nearestSampler);
						gl::Cmd::BindImage(0, *indirectUnfilteredTexPrev, 0);
						gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit);
						gl::Cmd::DispatchInvocations(workSize);

						for (int i = 1; i < 5 - std::log2f(float(inverseResolutionScale)); i++)
						{
							filterUniforms.stepWidth = (1 << i) * spatialFilterStep;

							filterUniforms.direction = { 0, 1 };
							filterUniformBuffer.UpdateData(filterUniforms);
							gl::Cmd::BindSampledImage(0, i == 1 ? *indirectUnfilteredTexPrev : *indirectFilteredTex, nearestSampler);
							gl::Cmd::BindImage(0, *indirectUnfilteredTex, 0);
							gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit);
							gl::Cmd::DispatchInvocations(workSize);

							filterUniforms.direction = { 1, 0 };
							filterUniformBuffer.UpdateData(filterUniforms);
							gl::Cmd::BindSampledImage(0, *indirectUnfilteredTex, nearestSampler);
							gl::Cmd::BindImage(0, *indirectFilteredTex, 0);
							gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit);
							gl::Cmd::DispatchInvocations(workSize);
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
							const gl::Texture* in{};
							const gl::Texture* out{};
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

							gl::Cmd::BindSampledImage(0, *in, nearestSampler);
							gl::Cmd::BindImage(0, *out, 0);
							gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit);
							gl::Cmd::DispatchInvocations(workSize);
						}
					}
				}

				auto& illuminationOutTex = inverseResolutionScale == 1 ? indirectFilteredTexPingPong : illuminationUpscaled;
				if (!rsmFilteredSkipAlbedoModulation)
				{
					// No upscale required
					if (inverseResolutionScale == 1)
					{
						gl::ScopedDebugMarker marker2("Modulate Albedo");

						gl::Cmd::BindComputePipeline(modulatePipeline);
						gl::Cmd::BindSampledImage(0, *indirectFilteredTex, nearestSampler);
						gl::Cmd::BindSampledImage(1, gAlbedo, nearestSampler);
						gl::Cmd::BindImage(0, *illuminationOutTex, 0);
						gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit);
						gl::Cmd::DispatchInvocations(illuminationOutTex->GetExtent());
					}
					else // Use bilateral upscale
					{
						gl::ScopedDebugMarker marker2("Modulate Albedo (Upscale)");

						gl::Cmd::BindComputePipeline(modulateUpscalePipeline);

						if (!useSeparableFilter && (5 - int(std::log2f(float(inverseResolutionScale)))) % 2 == 0)
						{
							gl::Cmd::BindSampledImage(0, *indirectUnfilteredTex, nearestSampler);
						}
						else
						{
							gl::Cmd::BindSampledImage(0, *indirectFilteredTex, nearestSampler);
						}

						gl::Cmd::BindSampledImage(1, gAlbedo, nearestSampler);
						gl::Cmd::BindSampledImage(2, gNormal, nearestSampler);
						gl::Cmd::BindSampledImage(3, gDepth, nearestSampler);
						gl::Cmd::BindSampledImage(4, *gNormalSmall, nearestSampler);
						gl::Cmd::BindSampledImage(5, *gDepthSmall, nearestSampler);
						filterUniforms.targetDim = { illuminationOutTex->GetExtent().width, illuminationOutTex->GetExtent().height };
						filterUniformBuffer.UpdateData(filterUniforms);
						gl::Cmd::BindUniformBuffer(0, filterUniformBuffer);
						gl::Cmd::BindImage(0, *illuminationOutTex, 0);
						gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit);
						gl::Cmd::DispatchInvocations(illuminationOutTex->GetExtent());
					}
				}
				else
				{
					gl::BlitTexture(*indirectFilteredTex,
						*illuminationOutTex,
						{},
						{},
						indirectFilteredTex->GetExtent(),
						illuminationOutTex->GetExtent(),
						gl::MagFilter::Nearest);
				}
			}
			else // Unfiltered RSM: the original paper
			{
				gl::Cmd::BindComputePipeline(rsmIndirectPipeline);
				gl::Cmd::BindSampledImage(1, gAlbedo, nearestSampler);
				gl::Cmd::BindSampledImage(2, gNormal, nearestSampler);
				gl::Cmd::BindSampledImage(3, gDepth, nearestSampler);
				gl::Cmd::BindSampledImage(4, rsmFlux, nearestSamplerClamped);
				gl::Cmd::BindSampledImage(5, rsmNormal, nearestSampler);
				gl::Cmd::BindSampledImage(6, rsmDepth, nearestSampler);
				gl::Cmd::BindSampledImage(0, *illuminationUpscaled, nearestSampler);
				gl::Cmd::BindImage(0, *illuminationUpscaled, 0);

				const auto workgroupSize = rsmIndirectFilteredPipeline.WorkgroupSize();
				const auto workSize =
					Extent3D{ (uint32_t)rsmUniforms.targetDim.x / 2, (uint32_t)rsmUniforms.targetDim.y / 2, 1 };

				// Quarter resolution indirect illumination pass
				rsmUniforms.currentPass = 0;
				rsmUniformBuffer.UpdateData(rsmUniforms);
				gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit);
				gl::Cmd::DispatchInvocations(workSize);

				// Reconstruction pass 1
				rsmUniforms.currentPass = 1;
				rsmUniformBuffer.UpdateData(rsmUniforms);
				gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit);
				gl::Cmd::DispatchInvocations(workSize);

				// Reconstruction pass 2
				rsmUniforms.currentPass = 2;
				rsmUniformBuffer.UpdateData(rsmUniforms);
				gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit);
				gl::Cmd::DispatchInvocations(workSize);

				// Reconstruction pass 3
				rsmUniforms.currentPass = 3;
				rsmUniformBuffer.UpdateData(rsmUniforms);
				gl::MemoryBarrier(gl::MemoryBarrierBit::TextureFetchBit);
				gl::Cmd::DispatchInvocations(workSize);
			}
		}
		gl::EndCompute();
	}

	gl::Texture& RsmTechnique::GetIndirectLighting()
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