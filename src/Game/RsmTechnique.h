#pragma once

namespace RSM
{
	struct CameraUniforms
	{
		glm::mat4 viewProj;
		glm::mat4 invViewProj;
		glm::mat4 proj;
		glm::vec4 cameraPos;
		glm::vec3 viewDir;
		uint32_t _padding00;
		glm::vec2 jitterOffset{};
		glm::vec2 lastFrameJitterOffset{};
	};

	class RsmTechnique
	{
	public:
		RsmTechnique(uint32_t width, uint32_t height);

		void SetResolution(uint32_t newWidth, uint32_t newHeight);

		// Input: camera uniforms, g-buffers, RSM buffers, previous g-buffer depth (for reprojection)
		void ComputeIndirectLighting(const glm::mat4& lightViewProj,
			const CameraUniforms& cameraUniforms,
			const gl4::Texture& gAlbedo,
			const gl4::Texture& gNormal,
			const gl4::Texture& gDepth,
			const gl4::Texture& rsmFlux,
			const gl4::Texture& rsmNormal,
			const gl4::Texture& rsmDepth,
			const gl4::Texture& gDepthPrev,
			const gl4::Texture& gNormalPrev,
			const gl4::Texture& gMotion);

		gl4::Texture& GetIndirectLighting();

		void DrawGui();

		int inverseResolutionScale = 1;
		int smallRsmSize = 512;
		int rsmSamples = 400;
		int rsmFilteredSamples = 8;
		float rMax = 0.2f;
		float spatialFilterStep = 1.0f;
		float alphaIlluminance = 0.05f;
		float phiNormal = 0.3f;
		float phiDepth = 0.2f;
		bool rsmFiltered = true;
		bool rsmFilteredSkipAlbedoModulation = false;
		bool seedEachFrame = true;
		bool useSeparableFilter = true;

	private:
		struct RsmUniforms
		{
			glm::mat4 sunViewProj;
			glm::mat4 invSunViewProj;
			glm::ivec2 targetDim;
			float rMax;
			uint32_t currentPass;
			uint32_t samples;
			uint32_t _padding00;
			glm::vec2 random;
		};

		struct ReprojectionUniforms
		{
			glm::mat4 invViewProjCurrent;
			glm::mat4 viewProjPrevious;
			glm::mat4 invViewProjPrevious;
			glm::mat4 proj;
			glm::vec3 viewPos;
			float temporalWeightFactor;
			glm::ivec2 targetDim;
			float alphaIlluminance;
			float phiDepth;
			float phiNormal;
			uint32_t _padding00;
			glm::vec2 jitterOffset;
			glm::vec2 lastFrameJitterOffset;
		};

		struct FilterUniforms
		{
			glm::mat4 proj;
			glm::mat4 invViewProj;
			glm::vec3 viewPos;
			float stepWidth;
			glm::ivec2 targetDim;
			glm::ivec2 direction;
			float phiNormal;
			float phiDepth;
			uint32_t _padding00;
			uint32_t _padding01;
		};

		uint32_t width;
		uint32_t height;
		uint32_t internalWidth;
		uint32_t internalHeight;
		glm::mat4 viewProjPrevious{ 1 };
		glm::uint seedX;
		glm::uint seedY;
		RsmUniforms rsmUniforms;
		gl4::TypedBuffer<RsmUniforms> rsmUniformBuffer;
		gl4::TypedBuffer<CameraUniforms> cameraUniformBuffer;
		gl4::TypedBuffer<ReprojectionUniforms> reprojectionUniformBuffer;
		gl4::TypedBuffer<FilterUniforms> filterUniformBuffer;
		gl4::ComputePipeline rsmIndirectPipeline;
		gl4::ComputePipeline rsmIndirectFilteredPipeline;
		gl4::ComputePipeline rsmReprojectPipeline;
		gl4::ComputePipeline bilateral5x5Pipeline;
		gl4::ComputePipeline modulatePipeline;
		gl4::ComputePipeline modulateUpscalePipeline;
		gl4::ComputePipeline blitPipeline;
		std::optional<gl4::Texture> indirectUnfilteredTex;
		std::optional<gl4::Texture> indirectUnfilteredTexPrev; // for temporal accumulation
		std::optional<gl4::Texture> indirectFilteredTex;
		std::optional<gl4::Texture> indirectFilteredTexPingPong;
		std::optional<gl4::Texture> historyLengthTex;
		std::optional<gl4::Texture> illuminationUpscaled;
		std::optional<gl4::Texture> rsmFluxSmall;
		std::optional<gl4::Texture> rsmNormalSmall;
		std::optional<gl4::Texture> rsmDepthSmall;
		std::optional<gl4::Texture> noiseTex;
		std::optional<gl4::Texture> gNormalSmall;
		std::optional<gl4::Texture> gDepthSmall;
		std::optional<gl4::Texture> gNormalPrevSmall;
		std::optional<gl4::Texture> gDepthPrevSmall;
	};
} // namespace RSM