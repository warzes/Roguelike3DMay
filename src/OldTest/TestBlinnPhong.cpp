#include "stdafx.h"
#include "TestBlinnPhong.h"
//=============================================================================
namespace
{
	const char* shaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// Interface block
out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	vs_out.FragPos = aPos; // Note that this should be multiplied by the model matrix
	vs_out.Normal = aNormal; // TODO transpose(inverse(mat3(model))) * aNormal;
	vs_out.TexCoords = aTexCoords;
	gl_Position = projection * view * vec4(aPos, 1.0);
}
)";

	const char* shaderCodeFragment = R"(
#version 460 core

out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

layout(binding = 0) uniform sampler2D floorTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
	vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;
	// ambient
	vec3 ambient = 0.05 * color;
	// diffuse
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 normal = normalize(fs_in.Normal);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * color;
	// specular
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;

	vec3 halfwayDir = normalize(lightDir + viewDir);  
	spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	vec3 specular = vec3(0.3) * spec; // assuming bright white light color
	FragColor = vec4(ambient + diffuse + specular, 1.0);
}
)";

	gl::ShaderProgramId program;
	int ViewLoc;
	int ProjLoc;
	int lightPosLoc;
	int viewPosLoc;

	GLuint texture;
	gl::BufferId vbo;
	gl::VertexArrayId vao;

	Camera camera;

	glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
}
//=============================================================================
EngineCreateInfo TestBlinnPhong::GetCreateInfo() const
{
	return {};
}
//=============================================================================
bool TestBlinnPhong::OnInit()
{
	program = gl::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);
	ViewLoc = gl::GetUniformLocation(program, "view");
	ProjLoc = gl::GetUniformLocation(program, "projection");
	lightPosLoc = gl::GetUniformLocation(program, "lightPos");
	viewPosLoc = gl::GetUniformLocation(program, "viewPos");

	texture = gl::LoadTexture2D("ExampleData/textures/wood.png", false);

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normals;
		glm::vec2 uv;
	};

	std::vector<gl::VertexAttributeRaw> attribs = {
		{0, 3, GL_FLOAT, false, offsetof(Vertex, pos)},
		{1, 3, GL_FLOAT, false, offsetof(Vertex, normals)},
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


	vbo = gl::CreateBufferStorage(0, sizeof(vertices), vertices);
	vao = gl::CreateVertexArray(vbo, sizeof(Vertex), attribs);

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	return true;
}
//=============================================================================
void TestBlinnPhong::OnClose()
{
	glDeleteTextures(1, &texture);
	gl::Destroy(program);
	gl::Destroy(vbo);
	gl::Destroy(vao);
}
//=============================================================================
void TestBlinnPhong::OnUpdate(float deltaTime)
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
		camera.ProcessMouseMovement(Input::GetCursorOffset().x, Input::GetCursorOffset().y);
	}
	else if (glfwGetMouseButton(GetGLFWWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		glfwSetInputMode(GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		Input::SetCursorVisible(true);
	}
}
//=============================================================================
void TestBlinnPhong::OnRender()
{
	gl::SetFrameBuffer({ 0 }, GetWindowWidth(), GetWindowHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), GetWindowAspect(), 0.01f, 1000.0f);

	// вывод квада
	{
		glUseProgram(program);
		gl::SetUniform(program, ViewLoc, view);
		gl::SetUniform(program, ProjLoc, proj);

		gl::SetUniform(program, lightPosLoc, lightPos);
		gl::SetUniform(program, viewPosLoc, camera.Position);

		glBindTextureUnit(0, texture);
		glBindVertexArray(vao);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}
//=============================================================================
void TestBlinnPhong::OnImGuiDraw()
{
}
//=============================================================================
void TestBlinnPhong::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================