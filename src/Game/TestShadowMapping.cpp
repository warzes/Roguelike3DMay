#include "stdafx.h"
#include "TestShadowMapping.h"
//=============================================================================
namespace
{
	const char* mainShaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT 
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
	vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
	vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
	vs_out.TexCoords = aTexCoords;
	vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

	const char* mainShaderCodeFragment = R"(
#version 460 core

out vec4 FragColor;

in VS_OUT 
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

// Shadow parameters
uniform float minBias; // 0.005
uniform float maxBias; // 0.05

// Light parameters
uniform float ambientPower; // 0.5
uniform int specularPower; // 32

float ShadowCalculation(vec4 fragPosLightSpace)
{
	// Perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
	// Transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	
	// Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r; 
	
	// Get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	
	// Calculate bias (based on depth map resolution and slope)
	vec3 normal = normalize(fs_in.Normal);
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	// Bias equation #1
	float bias = max(maxBias * (1.0 - dot(normal, lightDir)), minBias);
	// Bias equation #2
	//float cosTheta = clamp(dot(normal, lightDir), 0, 1);
	//float bias = 0.005 * tan(acos(cosTheta));
	//bias = clamp(bias, 0, 0.01);

	// Check whether current frag pos is in shadow
	// float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -3; x <= 3; ++x)
	{
		for(int y = -3; y <= 3; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
		}
	}
	shadow /= 49.0; // TODO create a uniform for blur radius

	// Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
	if(projCoords.z > 1.0)
	{
		shadow = 0.0;
	}
	return shadow;
}

void main()
{
	vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
	vec3 normal = normalize(fs_in.Normal);
	vec3 lightColor = vec3(0.6);

	// Ambient
	vec3 ambient = ambientPower * lightColor;
	
	// Diffuse
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * lightColor;
	
	// Specular
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	spec = pow(max(dot(normal, halfwayDir), 0.0), specularPower);
	vec3 specular = spec * lightColor;

	// Shadow
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

	FragColor = vec4(lighting, 1.0);
}
)";

	const char* lightShaderCodeVertex = R"(
#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 aPos;

layout(location = 0) out vec2 fragOffset;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 lightPosition;
uniform float radius;

const vec2 OFFSETS[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0,  1.0),
	vec2( 1.0, -1.0),
	vec2( 1.0, -1.0),
	vec2(-1.0,  1.0),
	vec2( 1.0,  1.0)
);

void main()
{
	vec3 camRight = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 camUp = vec3(view[0][1], view[1][1], view[2][1]);

	fragOffset = OFFSETS[gl_VertexID];

	vec3 positionWorld = lightPosition +
		radius * fragOffset.x * camRight +
		radius * fragOffset.y * camUp;
	
	gl_Position = projection * view * vec4(positionWorld, 1.0);
}
)";
	const char* lightShaderCodeFragment = R"(
#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragOffset;
layout(location = 0) out vec4 FragColor;

uniform vec3 lightColor;

void main()
{
	float dist = sqrt(dot(fragOffset, fragOffset));
    //float dist = dot(fragOffset, fragOffset); // Remove sqrt
	if (dist >= 1.0)
	{
		discard;
	}
	// Blurry edge
	float alpha = 1.0 - pow(dist, 5.0);
	FragColor = vec4(normalize(lightColor), alpha);
}
)";
	GLuint lightProgram;
	int lightProjLoc;
	int lightViewLoc;
	int lightPositionLoc;
	int radiusLoc;
	int lightColorLoc;

	GLuint mainProgram;
	int mainProjLoc;
	int mainViewLoc;
	int mainModelLoc;
	int mainLightSpaceMatrixLoc;
	int mainTextureDiffuse1Loc;
	int mainShadowMapLoc;
	int mainLightPosLoc;
	int mainViewPosLoc;
	int mainMinBiasLoc;
	int mainMaxBiasLoc;
	int mainAmbientPowerLoc;
	int mainSpecularPowerLoc;

	GLuint texture;

	Camera camera;

	Model* model;

	GLuint planevbo;
	GLuint planevao;

	PipelineShadowMapping* pipeline;

	// Light
	constexpr float lightY = 6.0f;
	constexpr float lightRadius = 5.0f;
	constexpr float lightSpeed = 0.1f;
	float lightTimer = 0.0f;
	bool moveLight = true;
	Light light(glm::vec3(0.f), glm::vec3(1.f));
	glm::vec3 target(0.f);

	// Shadow parameters
	float minBias = 0.005f;
	float maxBias = 0.05f;
	float shadowNearPlane = 1.0f;
	float shadowFarPlane = 20;

	// Light parameter
	int specularPower = 32;
	float ambientPower = 0.5;
}
//=============================================================================
EngineConfig TestShadowMapping::GetConfig() const
{
	return {};
}
//=============================================================================
bool TestShadowMapping::OnCreate()
{
	mainProgram = gl4::CreateShaderProgram(mainShaderCodeVertex, mainShaderCodeFragment);
	mainProjLoc = gl4::GetUniformLocation(mainProgram, "projection");
	mainViewLoc = gl4::GetUniformLocation(mainProgram, "view");
	mainModelLoc = gl4::GetUniformLocation(mainProgram, "model");
	mainLightSpaceMatrixLoc = gl4::GetUniformLocation(mainProgram, "lightSpaceMatrix");
	mainTextureDiffuse1Loc = gl4::GetUniformLocation(mainProgram, "texture_diffuse1");
	mainShadowMapLoc = gl4::GetUniformLocation(mainProgram, "shadowMap");
	mainLightPosLoc = gl4::GetUniformLocation(mainProgram, "lightPos");
	mainViewPosLoc = gl4::GetUniformLocation(mainProgram, "viewPos");
	mainMinBiasLoc = gl4::GetUniformLocation(mainProgram, "minBias");
	mainMaxBiasLoc = gl4::GetUniformLocation(mainProgram, "maxBias");
	mainAmbientPowerLoc = gl4::GetUniformLocation(mainProgram, "ambientPower");
	mainSpecularPowerLoc = gl4::GetUniformLocation(mainProgram, "specularPower");

	lightProgram = gl4::CreateShaderProgram(lightShaderCodeVertex, lightShaderCodeFragment);
	lightProjLoc = gl4::GetUniformLocation(lightProgram, "projection");
	lightViewLoc = gl4::GetUniformLocation(lightProgram, "view");
	lightPositionLoc = gl4::GetUniformLocation(lightProgram, "lightPosition");
	radiusLoc = gl4::GetUniformLocation(lightProgram, "radius");
	lightColorLoc = gl4::GetUniformLocation(lightProgram, "lightColor");

	texture = gl4::LoadTexture2D("data/textures/White1x1.png");

	model = new Model("data/mesh/Zaku/scene.gltf");

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	std::vector<gl4::VertexAttribute> attribs = {
		{0, 3, GL_FLOAT, false, offsetof(Vertex, pos)},
		{1, 3, GL_FLOAT, false, offsetof(Vertex, normal)},
		{2, 2, GL_FLOAT, false, offsetof(Vertex, uv)},
	};

	// Plane
	float planeVertices[]{
		// Positions			// Normals			// Texcoords
		 25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};

	planevbo = gl4::CreateBufferStorage(0, sizeof(planeVertices), planeVertices);
	planevao = gl4::CreateVertexArray(planevbo, sizeof(Vertex), attribs);


	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	constexpr int depthWidth = 2000;
	constexpr int depthHeight = 2000;
	pipeline = new PipelineShadowMapping(depthWidth, depthHeight);

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	return true;
}
//=============================================================================
void TestShadowMapping::OnDestroy()
{
	delete pipeline;
}
//=============================================================================
void TestShadowMapping::OnUpdate(float deltaTime)
{
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraForward, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraBackward, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraLeft, deltaTime);
	if (glfwGetKey(GetGLFWWindow(), GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(CameraRight, deltaTime);

	if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		SetCursorVisible(false);
		camera.ProcessMouseMovement(-GetMouseDeltaX(), -GetMouseDeltaY());
	}
	else if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		glfwSetInputMode(GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		SetCursorVisible(true);
	}

	// Calculate light position
	if (moveLight)
	{
		light.Position = glm::vec3(
			glm::sin(lightTimer) * lightRadius,
			lightY,
			glm::cos(lightTimer) * lightRadius);
		lightTimer += deltaTime * lightSpeed;
	}
}
//=============================================================================
void TestShadowMapping::OnRender()
{
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), GetAspect(), 0.01f, 1000.0f);

	// Generate depth map
	pipeline->StartRenderDepth(shadowNearPlane, shadowFarPlane, light.Position, target);
	auto depthShader = pipeline->GetDepthShader();
	auto depthModelLoc = pipeline->GetDepthShaderModelMatrixLoc();
	RenderScene(depthShader, depthModelLoc);

	// Render Scene
	gl4::SetFrameBuffer(0, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(mainProgram);
	gl4::SetUniform(mainTextureDiffuse1Loc, 0);
	gl4::SetUniform(mainShadowMapLoc, 1);
	gl4::SetUniform(mainMinBiasLoc, minBias);
	gl4::SetUniform(mainMaxBiasLoc, maxBias);
	gl4::SetUniform(mainAmbientPowerLoc, ambientPower);
	gl4::SetUniform(mainSpecularPowerLoc, specularPower);
	gl4::SetUniform(mainProjLoc, proj);
	gl4::SetUniform(mainViewLoc, view);
	gl4::SetUniform(mainViewPosLoc, camera.Position);
	gl4::SetUniform(mainLightPosLoc, light.Position);
	gl4::SetUniform(mainLightSpaceMatrixLoc, pipeline->GetLightSpaceMatrix());

	glBindTextureUnit(0, texture);
	pipeline->BindDepthTexture(1);

	RenderScene(mainProgram, mainModelLoc);

	// Debug light
	glUseProgram(lightProgram);
	gl4::SetUniform(lightProjLoc, proj);
	gl4::SetUniform(lightViewLoc, view);
	gl4::SetUniform(lightPositionLoc, light.Position);
	gl4::SetUniform(radiusLoc, 0.4f);
	gl4::SetUniform(lightColorLoc, light.Color);
	GetGraphicSystem().DrawQuad();

	// Debug depth
	//pipeline->DebugDrawDepthMap();
}
//=============================================================================
void TestShadowMapping::OnImGuiDraw()
{
	ImGui::SetNextWindowSize(ImVec2(380, 200));
	ImGui::Begin("Shadow Mapping");

	ImGui::SliderFloat("Min Bias", &minBias, 0.00001f, 0.01f);
	ImGui::SliderFloat("Max Bias", &maxBias, 0.001f, 0.1f);
	ImGui::SliderFloat("Shadow Near Plane", &shadowNearPlane, 0.1f, 5.0f);
	ImGui::SliderFloat("Shadow Far Plane", &shadowFarPlane, 10.0f, 500.0f);

	ImGui::Spacing();
	ImGui::Checkbox("Move Light", &moveLight);
	ImGui::SliderFloat("Ambient", &ambientPower, 0.01f, 1.0f);
	ImGui::SliderInt("Specular", &specularPower, 2, 128);

	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(60, 50));
	ImGui::Begin("FPS");
	ImGui::Text(std::to_string(GetFPS()).c_str());
	ImGui::End();
}
//=============================================================================
void TestShadowMapping::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================
void TestShadowMapping::RenderScene(GLuint shader, int modelMatLoc)
{
	glm::mat4 modelMat = glm::mat4(1.0f);
	gl4::SetUniform(modelMatLoc, modelMat);
	glBindVertexArray(planevao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	modelMat = glm::mat4(1.0f);
	modelMat = glm::translate(modelMat, glm::vec3(2.0f, 0.0f, 1.0f));
	gl4::SetUniform(modelMatLoc, modelMat);
	model->Draw(shader);

	modelMat = glm::mat4(1.0f);
	modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, 2.0f));
	gl4::SetUniform(modelMatLoc, modelMat);
	model->Draw(shader);

	modelMat = glm::mat4(1.0f);
	modelMat = glm::translate(modelMat, glm::vec3(-1.5f, 0.0f, 0.0f));
	gl4::SetUniform(modelMatLoc, modelMat);
	model->Draw(shader);
}
//=============================================================================