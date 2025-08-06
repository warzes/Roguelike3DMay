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
			const gl::Texture& gAlbedo,
			const gl::Texture& gNormal,
			const gl::Texture& gDepth,
			const gl::Texture& rsmFlux,
			const gl::Texture& rsmNormal,
			const gl::Texture& rsmDepth,
			const gl::Texture& gDepthPrev,
			const gl::Texture& gNormalPrev,
			const gl::Texture& gMotion);

		gl::Texture& GetIndirectLighting();

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
		gl::TypedBuffer<RsmUniforms> rsmUniformBuffer;
		gl::TypedBuffer<CameraUniforms> cameraUniformBuffer;
		gl::TypedBuffer<ReprojectionUniforms> reprojectionUniformBuffer;
		gl::TypedBuffer<FilterUniforms> filterUniformBuffer;
		gl::ComputePipeline rsmIndirectPipeline;
		gl::ComputePipeline rsmIndirectFilteredPipeline;
		gl::ComputePipeline rsmReprojectPipeline;
		gl::ComputePipeline bilateral5x5Pipeline;
		gl::ComputePipeline modulatePipeline;
		gl::ComputePipeline modulateUpscalePipeline;
		gl::ComputePipeline blitPipeline;
		std::optional<gl::Texture> indirectUnfilteredTex;
		std::optional<gl::Texture> indirectUnfilteredTexPrev; // for temporal accumulation
		std::optional<gl::Texture> indirectFilteredTex;
		std::optional<gl::Texture> indirectFilteredTexPingPong;
		std::optional<gl::Texture> historyLengthTex;
		std::optional<gl::Texture> illuminationUpscaled;
		std::optional<gl::Texture> rsmFluxSmall;
		std::optional<gl::Texture> rsmNormalSmall;
		std::optional<gl::Texture> rsmDepthSmall;
		std::optional<gl::Texture> noiseTex;
		std::optional<gl::Texture> gNormalSmall;
		std::optional<gl::Texture> gDepthSmall;
		std::optional<gl::Texture> gNormalPrevSmall;
		std::optional<gl::Texture> gDepthPrevSmall;
	};
} // namespace RSM