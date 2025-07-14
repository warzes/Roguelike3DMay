#pragma once

#include "TmpObject.h"

#define SHADOW_METHOD_PCF 0
#define SHADOW_METHOD_VSM 1
#define SHADOW_METHOD_ESM 2
#define SHADOW_METHOD_MSM 3

namespace tempApp
{
	struct SSAOConfig
	{
		bool enabled{ true };
		std::optional<gl4::Texture> texture{};
		std::optional<gl4::Texture> textureBlurred{};
		int samples_near{ 12 };
		int samples_mid{ 8 };
		int samples_far{ 4 };
		float near_extent{ 10.0f };
		float mid_extent{ 30.0f };
		float far_extent{ 70.0f };
		float delta{ .001f };
		float range{ 1.1f };
		float s{ 1.8f };
		float k{ 1.0f };
		float atrous_kernel[5] = { 0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f };
		float atrous_offsets[5] = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f };
		int atrous_passes{ 3 };
		float atrous_n_phi{ .1f };
		float atrous_p_phi{ .5f };
		float atrous_step_width{ 1.0f };
	};

	struct HDRConfig
	{
		std::optional<gl4::Texture> colorTex{};
		std::optional<gl4::Texture> depthTex{};
		std::optional<gl4::Buffer> histogramBuffer;
		std::optional<gl4::Buffer> exposureBuffer;
		float targetLuminance{ .22f };
		float minExposure{ .1f };
		float maxExposure{ 100.0f };
		float exposureFactor{ 1.0f };
		float adjustmentSpeed{ 2.0f };
		const int NUM_BUCKETS{ 128 };
	};

	struct SSRConfig
	{
		bool enabled{ false };
		std::optional<gl4::Texture> tex{};
		std::optional<gl4::Texture> texBlur{};
		GLuint framebuffer_width{ /*WINDOW_WIDTH / 2*/ };
		GLuint framebuffer_height{ /*WINDOW_HEIGHT / 2*/ };
		float rayStep{ 0.15f };
		float minRayStep{ 0.1f };
		float thickness{ 0.0f };
		float searchDist{ 15.0f };
		int maxRaySteps{ 30 };
		int binarySearchSteps{ 5 };
	};

	struct VolumetricConfig
	{
		bool enabled{ true };
		std::optional<gl4::Texture> tex{};
		std::optional<gl4::Texture> texBlur{};
		std::optional<gl4::Texture> atrousTex{};
		int atrous_passes = 1;
		GLint steps{ 32 };
		float intensity{ .025f };
		float noiseOffset{ 1.0f };
		float beerPower{ 1.0f };
		float powderPower{ 1.0f };

		float distanceScale{ 1.0f };
		float heightOffset{ 0.0f };
		float hfIntensity{ .025f };

		// a-trous filter stuff
		int atrousPasses{ 1 };
		float c_phi{ 0.04f };
		float stepWidth{ 1.0f };
		const float atrouskernel[25] = { // 5x5 gaussian kernel with std dev=1.75 (I think)
		1.0 / 256.0, 4.0 / 256.0, 6.0 / 256.0, 4.0 / 256.0, 1.0 / 256.0,
		4.0 / 256.0, 16.0 / 256.0, 24.0 / 256.0, 16.0 / 256.0, 4.0 / 256.0,
		6.0 / 256.0, 24.0 / 256.0, 36.0 / 256.0, 24.0 / 256.0, 6.0 / 256.0,
		4.0 / 256.0, 16.0 / 256.0, 24.0 / 256.0, 16.0 / 256.0, 4.0 / 256.0,
		1.0 / 256.0, 4.0 / 256.0, 6.0 / 256.0, 4.0 / 256.0, 1.0 / 256.0 };
		const glm::vec2 atrouskerneloffsets[25] = {
		  { -2, 2 }, { -1, 2 }, { 0, 2 }, { 1, 2 }, { 2, 2 },
		  { -2, 1 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, { 2, 1 },
		  { -2, 0 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 },
		  { -2, -1 }, { -1, -1 }, { 0, -1 }, { 1, -1 }, { 2, -1 },
		  { -2, -2 }, { -1, -2 }, { 0, -2 }, { 1, -2 }, { 2, -2 } };
	};
}

class TempApp final : public IEngineApp
{
public:
	TempApp() = default;
	TempApp(const TempApp&) = delete;
	TempApp(TempApp&&) = delete;
	void operator=(const TempApp&) = delete;
	void operator=(TempApp&&) = delete;

	EngineCreateInfo GetCreateInfo() const final;

	bool OnInit() final;
	void OnClose() final;
	void OnUpdate(float deltaTime) final;
	void OnRender() final;
	void OnImGuiDraw() final;
	void OnResize(uint16_t width, uint16_t height) final;
	void OnMouseButton(int button, int action, int mods) final;
	void OnMousePos(double x, double y) final;
	void OnScroll(double dx, double dy) final;
	void OnKey(int key, int scanCode, int action, int mods) final;

private:
	void createFramebuffers();

	// scene info
	Mesh sphere;
	Mesh sphere2;
	std::vector<ObjectBatched> batchedObjects;
	MaterialManager materialManager;
	GLuint legitFinalImage{};
	float magnifierScale{ .025f };
	bool magnifierLock{ false };
	glm::vec2 magnifier_lastMouse{};

	// lighting
	std::vector<PointLight> localLights;
	std::optional<gl4::Buffer> lightSSBO;
	float sunPosition{ 0 };
	DirLight globalLight;
	int numLights{ 1000 };
	glm::vec2 lightFalloff{ 2, 8 };
	float lightVolumeThreshold{ 0.01f };
	bool materialOverride{ false };
	glm::vec3 albedoOverride{ 0.129f, 0.643f, 0.921f };
	float roughnessOverride{ 0.5f };
	float metalnessOverride{ 1.0f };
	bool AOoverride{ false };
	float ambientOcclusionOverride{ 1.0f };

	gl4::Texture* bluenoiseTex;
	tempApp::SSAOConfig m_ssao;
	tempApp::HDRConfig m_hdr;
	tempApp::SSRConfig m_ssr;
	tempApp::VolumetricConfig m_volumetrics;

	// deferred stuff
	std::optional<gl4::Texture> gAlbedo{};
	std::optional<gl4::Texture> gNormal{};
	std::optional<gl4::Texture> m_gDepth{};
	std::optional<gl4::Texture> gRMA{}; // roughness, metalness, ambient occlusion
	std::optional<gl4::Texture> postprocessFbo{};
	std::optional<gl4::Texture> postprocessColor{};
	std::optional<gl4::Texture> postprocessPostSRGB{};

	// generic shadow stuff
	GLuint shadowFbo{};
	std::optional<gl4::Texture> shadowDepth{};
	GLuint SHADOW_WIDTH{ 1024 };
	GLuint SHADOW_HEIGHT{ 1024 };
	GLuint SHADOW_LEVELS{ (GLuint)glm::ceil(glm::log2((float)glm::max(SHADOW_WIDTH, SHADOW_HEIGHT))) };
	int BLUR_PASSES{ 1 };
	int BLUR_STRENGTH{ 5 };
	int shadow_method{ SHADOW_METHOD_ESM };
	bool shadow_gen_mips{ false };

	// variance shadow stuff
	GLuint vshadowGoodFormatFbo{};
	GLuint vshadowDepthGoodFormat{};
	GLuint vshadowMomentBlur{};
	float vlightBleedFix{ .9f };

	// exponential shadow stuff
	GLuint eShadowFbo{};
	GLuint eExpShadowDepth{};
	GLuint eShadowDepthBlur{};
	float eConstant{ 80.0f };

	// moment shadow stuff
	GLuint msmShadowFbo{};
	GLuint msmShadowMoments{};
	GLuint msmShadowMomentsBlur{};
	float msmA = 3e-5f; // unused

	GLint uiViewBuffer{};
};