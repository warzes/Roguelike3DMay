#include "stdafx.h"
#include "TestPBR.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	TexCoords = aTexCoords;
	WorldPos = vec3(model * vec4(aPos, 1.0));
	Normal = transpose(inverse(mat3(model))) * aNormal;

	gl_Position = projection * view * vec4(WorldPos, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// Material parameters
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metalness1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_ao1;
uniform sampler2D texture_emissive1;

// Lights
const int MAX_LIGHT_COUNT = 4;
uniform vec3 lightPositions[MAX_LIGHT_COUNT] ;
uniform vec3 lightColors[MAX_LIGHT_COUNT] ;

uniform vec3 camPos;

// PBR Functions in a sperate file
//#include PBR//pbr_include.fragment
//===================================================>
const float PI = acos(-1.0);

// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
vec3 GetNormalFromMap(vec3 tangentNormal, vec3 worldPos, vec3 normal, vec2 texCoords)
{
	vec3 Q1 = dFdx(worldPos);
	vec3 Q2 = dFdy(worldPos);
	vec2 st1 = dFdx(texCoords);
	vec2 st2 = dFdy(texCoords);

	vec3 N = normalize(normal);
	vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
//<===================================================

void main()
{
	vec3 albedo = texture(texture_diffuse1, TexCoords).rgb;
	vec3 emissive = texture(texture_emissive1, TexCoords).rgb;
	float metallic = texture(texture_metalness1, TexCoords).b; // Blue channel
	float roughness = texture(texture_roughness1, TexCoords).g; // Green channel
	float ao = texture(texture_ao1, TexCoords).r; // Red channel

	vec3 tangentNormal = texture(texture_normal1, TexCoords).xyz * 2.0 - 1.0;
	vec3 N = GetNormalFromMap(tangentNormal, WorldPos, Normal, TexCoords);

	vec3 V = normalize(camPos - WorldPos);

	// Calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	// Reflectance equation
	vec3 Lo = vec3(0.0);
	for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
	{
		// Calculate per-light radiance
		vec3 L = normalize(lightPositions[i] - WorldPos);
		vec3 H = normalize(V + L);
		float distance = length(lightPositions[i] - WorldPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColors[i] * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
		vec3 specular = numerator / denominator;

		// kS is equal to Fresnel
		vec3 kS = F;
		// for energy conservation, the diffuse and specular light can't
		// be above 1.0 (unless the surface emits light); to preserve this
		// relationship the diffuse component (kD) should equal 1.0 - kS.
		vec3 kD = vec3(1.0) - kS;
		// Multiply kD by the inverse metalness such that only non-metals 
		// have diffuse lighting, or a linear blend if partly metal (pure metals
		// have no diffuse light).
		kD *= 1.0 - metallic;

		// Scale light by NdotL
		float NdotL = max(dot(N, L), 0.0);

		// Add to outgoing radiance Lo
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	}

	// Ambient lighting (note that the next IBL tutorial will replace 
	// this ambient lighting with environment lighting).
	vec3 ambient = vec3(0.03) * albedo * ao;

	vec3 color = ambient + emissive + Lo;

	// HDR tonemapping
	color = color / (color + vec3(1.0));
	// Gamma correct
	color = pow(color, vec3(1.0 / 2.2));

	FragColor = vec4(color, 1.0);
}
)";

	const char* shaderLightCodeVertex = R"(
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

	const char* shaderLightCodeFragment = R"(
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

	gl::ShaderProgramId shader;
	gl::ShaderProgramId lightSphereShader;

	Camera camera;

	std::vector<LightOLD> lights;

	ModelOLD* renderModel1;
	ModelOLD* renderModel2;
}
//=============================================================================
EngineCreateInfo TestPBR::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool TestPBR::OnInit()
{
	shader = gl::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);
	lightSphereShader = gl::CreateShaderProgram(shaderLightCodeVertex, shaderLightCodeFragment);

	lights.emplace_back(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(150.0f, 40.0f, 40.0f));
	lights.emplace_back(glm::vec3(5.f, 0.0f, 5.0f), glm::vec3(40.0f, 150.0f, 40.0f));
	lights.emplace_back(glm::vec3(-5.f, 0.0f, 5.0f), glm::vec3(40.0f, 40.0f, 150.0f));

	//const float pi2 = glm::two_pi<float>();
	//for (uint32_t i = 0; i < 200; ++i)
	//{
	//	const float yPos = RandomNumber<float>(0.15f, 10.0f);
	//	const float radius = RandomNumber<float>(5.0f, 15.0f);
	//	const float rad = RandomNumber<float>(0.0f, pi2);
	//	float xPos = glm::cos(rad);

	//	glm::vec3 position(
	//		glm::cos(rad) * radius,
	//		yPos,
	//		glm::sin(rad) * radius
	//	);

	//	glm::vec3 color(
	//		RandomNumber<float>(0.5f, 0.8f),
	//		RandomNumber<float>(0.5f, 0.8f),
	//		RandomNumber<float>(0.7f, 1.0f)
	//	);

	//	lights.emplace_back(position, color);
	//}



	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	renderModel1 = new ModelOLD("ExampleData/mesh/DamagedHelmet/DamagedHelmet.gltf");
	renderModel2 = new ModelOLD("ExampleData/mesh/Tachikoma/Tachikoma.gltf");

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	gr.Create();

	return true;
}
//=============================================================================
void TestPBR::OnClose()
{
	gr.Destroy();
}
//=============================================================================
void TestPBR::OnUpdate(float deltaTime)
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
		Input::SetCursorVisible(false);
		camera.ProcessMouseMovement(-Input::GetScreenOffset().x, -Input::GetScreenOffset().y);
	}
	else if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		glfwSetInputMode(GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		Input::SetCursorVisible(true);
	}
}
//=============================================================================
void TestPBR::OnRender()
{
	gl::SetFrameBuffer({ 0 }, GetWindowWidth(), GetWindowHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), GetWindowAspect(), 0.01f, 1000.0f);

	// вывод квада
	{
		glUseProgram(shader);
		gl::SetUniform(shader, "projection", proj);
		gl::SetUniform(shader, "view", view);
		gl::SetUniform(shader, "camPos", camera.Position);

		for (unsigned int i = 0; i < lights.size(); ++i)
		{
			gl::SetUniform(shader, "lightPositions[" + std::to_string(i) + "]", lights[i].Position);
			gl::SetUniform(shader, "lightColors[" + std::to_string(i) + "]", lights[i].Color);
		}

		RenderScene(shader);

		glUseProgram(lightSphereShader);
		gl::SetUniform(lightSphereShader, "projection", proj);
		gl::SetUniform(lightSphereShader, "view", view);
		for (auto& l : lights)
		{
			gl::SetUniform(lightSphereShader, "lightPosition", l.Position);
			gl::SetUniform(lightSphereShader, "radius", 0.4f);
			gl::SetUniform(lightSphereShader, "lightColor", l.Color);
			gr.DrawQuad();
		}

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}
//=============================================================================
void TestPBR::OnImGuiDraw()
{
	ImGui::SetNextWindowSize(ImVec2(60, 50));
	ImGui::Begin("FPS");
	ImGui::Text(std::to_string(GetFPS()).c_str());
	ImGui::End();
}
//=============================================================================
void TestPBR::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================
void TestPBR::RenderScene(gl::ShaderProgramId shader)
{
	bool skipTextureBinding = false;

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	gl::SetUniform(shader, "model", modelMatrix);
	renderModel1->Draw(shader, skipTextureBinding);

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.5f, 0.0f, 0.0f));
	gl::SetUniform(shader, "model", modelMatrix);
	renderModel2->Draw(shader, skipTextureBinding);
}
//=============================================================================