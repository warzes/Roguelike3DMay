#include "stdafx.h"
#include "TestComplex.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexCoords;

out VS_DATA
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vsOut;

layout(std140, binding = 0) uniform MVPData {
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

void main()
{
	vec4 worldSpacePosition = modelMatrix * vec4(aVertexPosition, 1.0);
	gl_Position = projectionMatrix * viewMatrix * worldSpacePosition;

	vsOut.FragPos = worldSpacePosition.xyz;
	vsOut.Normal = transpose(inverse(mat3(modelMatrix))) * aVertexNormal;
	vsOut.TexCoords = aVertexTexCoords;
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

in VS_DATA
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fsIn;

layout(binding = 0) uniform sampler2D diffuseTexture1;

out vec4 FragColorOut;

void main()
{
	FragColorOut = texture(diffuseTexture1, fsIn.TexCoords);
}
)";

	struct alignas(16) MVPData final
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
	};

	gl4::ShaderProgram program;

	MVPData mvpData;
	GLuint mvpUbo;

	GLuint texture;
	gl4::Buffer vbo;
	gl4::VertexArray vao;

	Camera camera;

	Scene scene;
	SceneDesc* scene_desc;
	Camera2 active_camera;
	CameraControl camera_control;
	ShadowMapper* shadow_map_point_light;

	Deferred* deferred;

	std::vector<GLfloat> rnds;
	std::uniform_real_distribution<GLfloat> random_float(0.0, 1.0);
	std::default_random_engine generator;
	std::vector<float> screen_rnd;

	Texture2D* screen_rnd_tex;

	UniformBuffer* rnd_ubo;

	Shader* ssao_shader;
	Texture2D* ssao_texture;
	FramebufferObject* ssao_fbo;

	Shader* lighting_shader;
	Texture2D* lighting_tex;
	FramebufferObject* lighting_fbo;

	Shader* rsm_shader;
	Texture2D* rsm_tex;
	FramebufferObject* rsm_fbo;

	Shader* ssr_shader;
	Texture2D* ssr_tex;
	FramebufferObject* ssr_fbo;

	Shader* post_shader;
}
//=============================================================================
EngineConfig TestComplex::GetConfig() const
{
	return {};
}
//=============================================================================
bool TestComplex::OnCreate()
{
	program = gl4::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);

	mvpUbo = gl4::CreateBufferStorage(GL_DYNAMIC_STORAGE_BIT, sizeof(MVPData), nullptr);

	texture = gl4::LoadTexture2D("ExampleData/textures/wood.png", false);

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

	// Quad
	float vertices[]{
		// positions            // normals            // texcoords
		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
		 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
	};

	vbo = gl4::CreateBufferStorage(0, sizeof(vertices), vertices);
	vao = gl4::CreateVertexArray(vbo, sizeof(Vertex), attribs);

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	scene.add(std::make_shared<Model2>("ExampleData/mesh/Sponza/Sponza.gltf"), glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)));
	scene.add(std::make_shared<PointLight>(glm::vec3(17.0f, 13.0f, 4.0f) * 300.0f, glm::vec3(0.0f, 20.0f, -3.0f), 3.0f));
	// scene.add(std::make_shared<Model>("mitsuba.obj"));
	// scene.add(std::make_shared<Model>("spot.obj"), glm::translate(glm::mat4(1.0f), glm::vec3(-1.8f, 0.7f, 0.0f)));
	// scene.add(std::make_shared<PointLight>(glm::vec3(500.0f, 500.0f, 500.0f), glm::vec3(-15.0f, 10.0f, 0.0f), 1.0f));
	// scene.add(std::make_shared<PointLight>(glm::vec3(50.0f, 30.1f, 0.1f), glm::vec3(0.5f, 0.3f, 10.0f), 1.0f));
	scene.ambient_light_irradiance = glm::vec3(0.06f, 0.08f, 0.10f);

	scene_desc = new SceneDesc(scene);
	
	active_camera.aspect = 1.0f * GetAspect();
	camera_control.bind(&active_camera);

	shadow_map_point_light = new ShadowMapper();

	deferred = new Deferred(GetWidth(), GetHeight());

	for (int i = 0; i < GetWidth() * GetHeight(); i++)
	{
		screen_rnd.push_back(random_float(generator));
	}
	screen_rnd_tex = new Texture2D(GetWidth(), GetHeight(), screen_rnd.data(), GL_R16F, GL_RED, GL_FLOAT);

	for (int i = 0; i < 1024; i++)
	{
		rnds.push_back(random_float(generator));
	}

	rnd_ubo = new UniformBuffer;
	rnd_ubo->setData(4096, rnds.data(), GL_STATIC_DRAW);
	rnd_ubo->use(1);

	ssao_shader = new Shader("ExampleData/shaders/TestComplex/ssao.vert", "ExampleData/shaders/TestComplex/ssao.frag");
	ssao_texture = new Texture2D(GetWidth(), GetHeight());
	ssao_fbo = new FramebufferObject({ ssao_texture }, GetWidth(), GetHeight());

	lighting_shader = new Shader("ExampleData/shaders/TestComplex/lighting.vs", "ExampleData/shaders/TestComplex/lighting.fs");
	lighting_tex = new Texture2D(GetWidth(), GetHeight());
	lighting_fbo = new FramebufferObject({ lighting_tex }, GetWidth(), GetHeight());

	rsm_shader = new Shader("ExampleData/shaders/TestComplex/rsm.vs", "ExampleData/shaders/TestComplex/rsm.fs");
	rsm_tex = new Texture2D(GetWidth(), GetHeight());
	rsm_fbo = new FramebufferObject({ rsm_tex }, GetWidth(), GetHeight());

	ssr_shader = new Shader("ExampleData/shaders/TestComplex/ssr.vs", "ExampleData/shaders/TestComplex/ssr.fs");
	ssr_tex = new Texture2D(GetWidth(), GetHeight());
	ssr_fbo = new FramebufferObject({ ssr_tex }, GetWidth(), GetHeight());

	post_shader = new Shader("ExampleData/shaders/TestComplex/post.vert", "ExampleData/shaders/TestComplex/post.frag");

	lighting_shader->use();
	lighting_shader->setTexture("screen_rnd_tex", screen_rnd_tex);
	lighting_shader->setUniblock("ub_common", 1);
	shadow_map_point_light->attach(*lighting_shader);

	ssao_shader->use();
	ssao_shader->setTexture("screen_rnd_tex", screen_rnd_tex);
	ssao_shader->setUniblock("ub_common", 1);

	rsm_shader->use();
	rsm_shader->setTexture("screen_rnd_tex", screen_rnd_tex);
	rsm_shader->setUniblock("ub_common", 1);
	shadow_map_point_light->attach(*rsm_shader);

	ssr_shader->use();
	ssr_shader->setTexture("screen_rnd_tex", screen_rnd_tex);
	ssr_shader->setUniblock("ub_common", 1);

	lighting_shader->setTexture("ao", ssao_texture);
	ssr_shader->setTexture("film", lighting_tex);
	post_shader->setTexture("lighting", lighting_tex);
	post_shader->setTexture("rsm", rsm_tex);
	post_shader->setTexture("ssr", ssr_tex);

	return true;
}
//=============================================================================
void TestComplex::OnDestroy()
{
}
//=============================================================================
void TestComplex::OnUpdate(float deltaTime)
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
}
//=============================================================================
void TestComplex::OnRender()
{
	camera_control.onEvents();

	// SHADOW STAGE
	shadow_map_point_light->lightPass(scene_desc->point_lights[0].position, scene_desc->point_lights[0].intensity, &scene_desc->models);
	glViewport(0, 0, GetWidth(), GetHeight());

	// GEOMETRY STAGE
	deferred->drawGeometry(&scene_desc->models, active_camera);

	// SSAO STAGE
	ssao_fbo->use();
	ssao_shader->use();
	ssao_shader->setCamera(active_camera);
	deferred->drawLighting(*ssao_shader);

	// LIGHTING STAGE
	lighting_fbo->use();
	lighting_shader->use();
	lighting_shader->setLights(scene_desc->point_lights);
	lighting_shader->setUniform("ambient", scene_desc->ambient_light_irradiance);
	lighting_shader->setCamera(active_camera);
	deferred->drawLighting(*lighting_shader);

	// RSM STAGE
	rsm_fbo->use();
	rsm_shader->use();
	rsm_shader->setLights(scene_desc->point_lights);
	rsm_shader->setUniform("ambient", scene_desc->ambient_light_irradiance);
	rsm_shader->setCamera(active_camera);
	deferred->drawLighting(*rsm_shader);

	// SSR STAGE
	ssr_fbo->use();
	ssr_shader->use();
	ssr_shader->setCamera(active_camera);
	deferred->drawLighting(*ssr_shader);

	// POST PROCESSING
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	deferred->drawLighting(*post_shader);


	//gl4::SetFrameBuffer({ 0 }, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//mvpData.model = glm::mat4(1.0f);
	//mvpData.view = camera.GetViewMatrix();
	//mvpData.projection = glm::perspective(glm::radians(60.0f), GetAspect(), 0.01f, 1000.0f);
	//glNamedBufferSubData(mvpUbo, 0, sizeof(MVPData), &mvpData);

	//// вывод квада плоскости
	//{
	//	glUseProgram(program);
	//	glBindBufferBase(GL_UNIFORM_BUFFER, 0, mvpUbo);
	//	glBindTextureUnit(0, texture);
	//	glBindVertexArray(vao);

	//	glDrawArrays(GL_TRIANGLES, 0, 6);
	//}
}
//=============================================================================
void TestComplex::OnImGuiDraw()
{
	/*ImGui::Begin("Simple");

	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char*)glGetString(GL_VENDOR));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char*)glGetString(GL_VERSION));
	ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char*)glGetString(GL_RENDERER));
	ImGui::Separator();

	ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();*/
}
//=============================================================================
void TestComplex::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================