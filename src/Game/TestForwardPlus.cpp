#include "stdafx.h"
#include "TestForwardPlus.h"

// TODO: выделить стадию рендера в глубину, так как она универсальна
//=============================================================================
namespace
{
	Camera camera;
	Model* model;

#pragma region Forward+

#pragma region DepthShader
	const char* depthShaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 aVertexPosition;

// Uniforms
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
	gl_Position = projection * view * model * vec4(aVertexPosition, 1.0);
}
)";

	const char* depthShaderCodeFragment = R"(
#version 460 core

void main()
{
}
)";
#pragma endregion

#pragma region depthDebugShader
	const char* depthDebugShaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0);
	TexCoords = texCoords;
}
)";

	const char* depthDebugShaderCodeFragment = R"(
#version 460 core

uniform float near;
uniform float far;

out vec4 fragColor;

// Need to linearize the depth because we are using the projection
float LinearizeDepth(float depth) {
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
	float depth = LinearizeDepth(gl_FragCoord.z) / far;
	fragColor = vec4(vec3(depth), 1.0f);
}
)";
#pragma endregion

#pragma region lightCullingShaderCompute

	const char* lightCullingShaderCodeCompute = R"(
#version 460 core

#define MAX_LIGHTS_PER_TILE 128
#define TILE_SIZE 16

struct PointLight {
	vec4 position;
	vec4 color;
	vec4 paddingAndRadius; // .w содержит радиус
};

// Shader storage buffer objects
layout(std430, binding = 0) readonly buffer LightBuffer {
	PointLight data[];
};

layout(std430, binding = 1) writeonly buffer visible_lights_indices {
	int lights_indices[];
};

// Uniforms
uniform sampler2D depthMap;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 screenSize;
uniform int lightCount;

// Shared values
shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visibleLightCount;
shared vec4 frustumPlanes[6];
shared int visibleLightIndices[MAX_LIGHTS_PER_TILE];

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE) in;

void main() {

	ivec2 tile_id = ivec2(gl_WorkGroupID.xy);

	// Инициализация разделяемых данных одним потоком
	if (gl_LocalInvocationIndex == 0) {
		minDepthInt = 0xFFFFFFFF;
		maxDepthInt = 0;
		visibleLightCount = 0;

		uint index = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
		uint offset = index * MAX_LIGHTS_PER_TILE;
		for (uint i = 0; i < MAX_LIGHTS_PER_TILE; i++) {
			lights_indices[offset + i] = -1;
		}
	}
	barrier();

	// Чтение глубины и обновление мин/макс
	vec2 screen_uv = (vec2(gl_GlobalInvocationID.xy) + 0.5) / screenSize;
	float depth = texture(depthMap, screen_uv).r;
	uint depth_uint = floatBitsToUint(depth);

	atomicMin(minDepthInt, depth_uint);
	atomicMax(maxDepthInt, depth_uint);

	barrier();

	// Расчёт фрустума на одном потоке
	if (gl_LocalInvocationIndex == 0) {
		float min_z = uintBitsToFloat(minDepthInt);
		float max_z = uintBitsToFloat(maxDepthInt);

		vec4 vs_min_depth = inverse(projection) * vec4(0.0, 0.0, 2.0 * min_z - 1.0, 1.0);
		vec4 vs_max_depth = inverse(projection) * vec4(0.0, 0.0, 2.0 * max_z - 1.0, 1.0);
		vs_min_depth /= vs_min_depth.w;
		vs_max_depth /= vs_max_depth.w;

		float nearZ = vs_min_depth.z;
		float farZ = vs_max_depth.z;

		vec2 tileScale = screenSize / (2.0 * float(TILE_SIZE));
		vec2 tileBias = vec2(gl_WorkGroupID.xy) - tileScale;

		vec4 col1 = vec4(-projection[0][0] * tileScale.x, projection[0][1], tileBias.x, projection[0][3]);
		vec4 col2 = vec4(projection[1][0], -projection[1][1] * tileScale.y, tileBias.y, projection[1][3]);
		vec4 col3 = vec4(projection[3][0], projection[3][1], -1.0, projection[3][3]);

		frustumPlanes[0] = col3 + col1;
		frustumPlanes[1] = col3 - col1;
		frustumPlanes[2] = col3 - col2;
		frustumPlanes[3] = col3 + col2;
		frustumPlanes[4] = vec4(0.0, 0.0, 1.0, -nearZ);
		frustumPlanes[5] = vec4(0.0, 0.0, -1.0, farZ);

		// Нормализация только для плоскостей отсечения
		for (uint i = 0; i < 4; i++) {
			frustumPlanes[i] *= 1.0f / length(frustumPlanes[i].xyz);
		}
	}

	barrier();

	// Куллинг освещения
	uint threadCount = TILE_SIZE * TILE_SIZE;
	for (uint i = gl_LocalInvocationIndex;i < uint(lightCount); i += threadCount)
	{
		PointLight light = data[i];
		vec4 vs_light_pos = view * vec4(light.position.xyz, 1.0);

		bool inFrustum = true;
		for (uint j = 0; j < 6 && inFrustum; j++)
		{
			float distance = dot(frustumPlanes[j], vs_light_pos);
			inFrustum = (distance >= -light.paddingAndRadius.w);
		}
		if (inFrustum)
		{
			uint idx = atomicAdd(visibleLightCount, 1u);
			if (idx < MAX_LIGHTS_PER_TILE)
			{
				visibleLightIndices[idx] = int(i);
			}
		}
	}

	barrier();

	// Запись результата
	if (gl_LocalInvocationIndex == 0)
	{
		uint baseIndex = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
		uint offset = baseIndex * MAX_LIGHTS_PER_TILE;
		for (uint i = 0; i < visibleLightCount; i++)
		{
			lights_indices[offset + i] = visibleLightIndices[i];
		}
	}
}
)";
#pragma endregion

// TODO: почему тормозит
#pragma region lightingShader

	const char* lightingShaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec3 vert_normal;
layout (location = 2) in vec2 vert_uv;
layout (location = 3) in vec3 vert_tangent;

out VERTEX_OUT {
  vec2 frag_uv;
  mat3 TBN;
  vec3 ts_frag_pos;
  vec3 ts_view_pos;
} vertex_out;

// Uniforms
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 viewPosition;

void main() {
	gl_Position = projection * view* model * vec4(vert_pos, 1.0);
	vec3 frag_pos = vec3(model * vec4(vert_pos, 1.0));

	mat3 normal_matrix = transpose(inverse(mat3(model)));
	vec3 N = normalize(vec3(normal_matrix * vert_normal));
	vec3 T = normalize(vec3(normal_matrix * vert_tangent));
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	vertex_out.frag_uv = vert_uv;
	vertex_out.TBN = transpose(mat3(T, B, N));    
	vertex_out.ts_view_pos  = vertex_out.TBN * viewPosition;
	vertex_out.ts_frag_pos = vertex_out.TBN * frag_pos;
}
)";

	const char* lightingShaderCodeFragment = R"(
#version 460 core

#define MAX_LIGHTS_PER_TILE 128
#define TILE_SIZE 16

in VERTEX_OUT{
	vec2 frag_uv;
	mat3 TBN;
	vec3 ts_frag_pos;
	vec3 ts_view_pos;
} fragment_in;

struct PointLight {
	vec4 position;
	vec4 color;
	vec4 paddingAndRadius; // .w содержит радиус света
};

// Shader storage buffer objects
layout(std430, binding = 0) buffer LightBuffer {
	PointLight data[];
} lightBuffer;

layout(std430, binding = 1) buffer visible_lights_indices {
	int lights_indices[];
};

// Uniforms
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

uniform int doLightDebug;
uniform int numberOfTilesX;

out vec4 fragColor;

void main() {
	// Determine which tile this pixel belongs to
	ivec2 tileID = ivec2(gl_FragCoord.xy) / TILE_SIZE;
	uint index = tileID.y * numberOfTilesX + tileID.x;
    uint offset = index * MAX_LIGHTS_PER_TILE;

	vec4 base_diffuse = texture(texture_diffuse1, fragment_in.frag_uv);
	if (base_diffuse.a <= 0.2) discard;

	if (doLightDebug==1)
	{
		uint count = 0;
		for (uint i = 0; i < MAX_LIGHTS_PER_TILE; i++)
		{
			if (lights_indices[offset + i] != -1 ) {
				count++;
			}
		}
		float shade = float(count) / float(MAX_LIGHTS_PER_TILE * 2); 
		fragColor = vec4(shade);
		return;
	}

	vec3 normal = normalize((texture(texture_normal1, fragment_in.frag_uv).rgb * 2.0 - 1.0));
	float specpower = 60.0f;

	vec3 result = vec3(0.0);

	for (uint i = 0; i < MAX_LIGHTS_PER_TILE; i++)
	{
		int idx = lights_indices[offset + i];
		if (idx == -1) continue;

		PointLight light = lightBuffer.data[idx];

		vec3 lightPosTS = fragment_in.TBN * light.position.xyz;
		vec3 lightDirTS = lightPosTS - fragment_in.ts_frag_pos;
		float dist = length(lightDirTS);
		float attenuation = clamp(1.0 - dist * dist / (light.paddingAndRadius.w * light.paddingAndRadius.w), 0.0, 1.0);
		if (attenuation == 0.0)
			continue;

		lightDirTS /= dist; // normalize

		float NdotL = max(0.0, dot(normal, lightDirTS));
		if (NdotL == 0.0)
			continue;

		// Диффузное освещение
		vec3 diffuse = light.color.rgb * base_diffuse.rgb * NdotL * attenuation;

		// Спекулярное освещение
		vec3 viewDirTS = normalize(fragment_in.ts_view_pos - fragment_in.ts_frag_pos);
		vec3 reflectDir = reflect(-lightDirTS, normal);
		float spec = pow(max(dot(viewDirTS, reflectDir), 0.0), specpower);
		vec3 specular = vec3(1.0) * spec * attenuation;

		result += diffuse + specular;
	}

	
	fragColor =vec4(result, 1.0);
}
)";

#pragma endregion

#pragma region FinalShader

const char* finalShaderCodeVertex = R"(
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textureCoordinates;

out vec2 TextureCoordinates;

void main() {
	gl_Position = vec4(position, 1.0);
	TextureCoordinates = textureCoordinates;
}
)";

const char* finalShaderCodeFragment = R"(
#version 460 core

// can be used for HDR

in vec2 TextureCoordinates;

uniform sampler2D colorBuffer;

out vec4 fragColor;


void main() {
	vec3 color = texture(colorBuffer, TextureCoordinates).rgb;
	fragColor = vec4(color, 1.0);
}
)";
#pragma endregion

	struct alignas(16) LightData final
	{
		glm::vec4 position;
		glm::vec4 color;
		glm::vec4 paddingAndRadius;
	};

	GLuint depthProgram;
	GLuint depthDebugProgram;
	GLuint lightCullingProgram;
	GLuint lightProgram;
	GLuint renderProgram;

	int numberOfLights{ 40 };

	GLuint lightBuffer; // lightBufferSSBO
	GLuint indexBuffer; // lightIndexBufferSSBO

	float nearPlane = 0.5f;
	float farPlane = 300.0f;


#define MAX_LIGHTS_PER_TILE 128
#define TILE_SIZE 16

	const glm::vec3 minLightBoundaries{ -5.0f, -5.0f, -25.0f };
	const glm::vec3 maxLightBoundaries{ 5.0f, 10.0f, 25.0f };

	GLuint workGroupsX;
	GLuint workGroupsY;


	GLuint depthMapFBO;
	GLuint depthMap;
	GLuint renderFBO;
	GLuint renderDepthBuffer;
	GLuint colorBuffer;

	enum Modes
	{
		SHADED,
		DEPTH,
		LIGHT
	} ViewModes;


	void SetupLights()
	{
		//glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfLights * sizeof(Light), NULL, GL_DYNAMIC_DRAW);

		if (lightBuffer == 0) return;

		//glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfLights * sizeof(LightData), NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
		LightData* lights = (LightData*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

		//	LightData* lights = (LightData*)glMapNamedBufferRange(lightBufferSSBO, 0, numberOfLights * sizeof(LightData), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		for (size_t i = 0; i < numberOfLights; i++)
		{
			LightData& light = lights[i];

			light.position.x = minLightBoundaries.x + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxLightBoundaries.x - minLightBoundaries.x)));
			light.position.y = minLightBoundaries.y + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxLightBoundaries.y - minLightBoundaries.y)));
			light.position.z = minLightBoundaries.z + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxLightBoundaries.z - minLightBoundaries.z)));
			light.position.w = 1.0f;

			light.color = {
				static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
				static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
				static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
				1.0f
			};

			light.paddingAndRadius = { 0.0f, 0.0f, 0.0f, 5.0f };
		}

		//glUnmapNamedBuffer(lightBufferSSBO);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void UpdateLights()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);

		LightData* lights = (LightData*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

		for (int i = 0; i < numberOfLights; i++)
		{
			LightData& light = lights[i];

			float min = minLightBoundaries[1];
			float max = maxLightBoundaries[1];

			light.position += glm::vec4(0, 0.2f, 0, 0);

			if (light.position[1] > maxLightBoundaries[1])
			{
				light.position[1] -= (maxLightBoundaries[1] - minLightBoundaries[1]);
			}
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

#pragma endregion

}
//=============================================================================
EngineConfig TestForwardPlus::GetConfig() const
{
	return {};
}
//=============================================================================
bool TestForwardPlus::OnCreate()
{
	model = new Model("data/mesh/Sponza/Sponza.gltf");
	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

#pragma region Forward+

	ViewModes = Modes::SHADED;

	workGroupsX = (GetWidth() + (GetWidth() % TILE_SIZE)) / TILE_SIZE;
	workGroupsY = (GetHeight() + (GetHeight() % TILE_SIZE)) / TILE_SIZE;

	GLuint tilesCount = workGroupsX * workGroupsY;

	depthProgram = gl4::CreateShaderProgram(depthShaderCodeVertex, depthShaderCodeFragment);
	depthDebugProgram = gl4::CreateShaderProgram(depthDebugShaderCodeVertex, depthDebugShaderCodeFragment);
	lightCullingProgram = gl4::CreateShaderProgram(lightCullingShaderCodeCompute);
	lightProgram = gl4::CreateShaderProgram(lightingShaderCodeVertex, lightingShaderCodeFragment);
	renderProgram = gl4::CreateShaderProgram(finalShaderCodeVertex, finalShaderCodeFragment);

	glUseProgram(depthProgram);
	glUniformMatrix4fv(glGetUniformLocation(depthProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

	glUseProgram(depthDebugProgram);
	glUniformMatrix4fv(glGetUniformLocation(depthDebugProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	glUniform1f(glGetUniformLocation(depthDebugProgram, "near"), nearPlane);
	glUniform1f(glGetUniformLocation(depthDebugProgram, "far"), farPlane);


	glUseProgram(lightCullingProgram);
	glUniform1i(glGetUniformLocation(lightCullingProgram, "lightCount"), numberOfLights);
	glUniform2fv(glGetUniformLocation(lightCullingProgram, "screenSize"), 1, glm::value_ptr(glm::vec2(GetWidth(), GetHeight())));


	glUseProgram(lightProgram);
	glUniformMatrix4fv(glGetUniformLocation(lightProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	glUniform1i(glGetUniformLocation(lightProgram, "numberOfTilesX"), workGroupsX);
	glUniform1i(glGetUniformLocation(lightProgram, "doLightDebug"), 0);
	glUniform3fv(glGetUniformLocation(lightProgram, "viewPosition"), 1, glm::value_ptr(camera.Position));

	// create light buffer
	//lightBufferSSBO = gl4::CreateBuffer(GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, numberOfLight * sizeof(LightData), nullptr);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBufferSSBO);
	//// create visible light indices buffer
	//lightIndexBufferSSBO = gl4::CreateBuffer(GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, sizeof(int) * tilesCount * MAX_LIGHTS_PER_TILE, nullptr);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightIndexBufferSSBO);

	// Create light buffer
	glGenBuffers(1, &lightBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfLights * sizeof(LightData), 0, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	// Create visible light indices buffer
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int)* workGroupsX* workGroupsY* MAX_LIGHTS_PER_TILE, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indexBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Generate lights
	SetupLights();
	
	// init framebuffer
	{
		// Depth buffer 
		glGenFramebuffers(1, &depthMapFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GetWidth(), GetHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glEnable(GL_DEPTH_TEST);

		// Output buffer + color
		glGenFramebuffers(1, &renderFBO);

		glGenTextures(1, &colorBuffer);
		glBindTexture(GL_TEXTURE_2D, colorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, GetWidth(), GetHeight(), 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// render depth
		glGenRenderbuffers(1, &renderDepthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderDepthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GetWidth(), GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderDepthBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
#pragma endregion

	glClearColor(0.7f, 0.8f, 0.9f, 1.0f);

	//glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}
//=============================================================================
void TestForwardPlus::OnDestroy()
{
}
//=============================================================================
void TestForwardPlus::OnUpdate(float deltaTime)
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
void TestForwardPlus::OnRender()
{
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), GetAspect(), nearPlane, farPlane);
	
	// вывод модели используя forward+
	{
		static const GLuint uint_zeros[] = { 0, 0, 0, 0 };
		static const GLfloat float_zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		static const GLfloat float_ones[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		static const GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

		UpdateLights();
		//UpdateSSBO();
		{
			GLuint workgroup_x = (GetWidth() + (GetWidth() % TILE_SIZE)) / TILE_SIZE;
			GLuint workgroup_y = (GetHeight() + (GetHeight() % TILE_SIZE)) / TILE_SIZE;

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexBuffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * workgroup_x * workgroup_y * MAX_LIGHTS_PER_TILE, NULL, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, GetWidth(), GetHeight());
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#pragma region DEPTH_FBO
		{
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_DEPTH_BUFFER_BIT);
			glDepthFunc(GL_LESS);

			//update depth uniforms
			glViewport(0, 0, GetWidth(), GetHeight());

			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(4.0f, 4.0f);

			glUseProgram(depthProgram);
			static const GLenum buffs[] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, buffs);
			glClearBufferfv(GL_COLOR, 0, float_zeros);
			glClearBufferfv(GL_DEPTH, 0, float_ones);

			glUniformMatrix4fv(glGetUniformLocation(depthProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(glGetUniformLocation(depthProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(depthProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

			model->Draw(depthProgram);
			glActiveTexture(GL_TEXTURE0); // TODO: удалить

			glDisable(GL_POLYGON_OFFSET_FILL);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
#pragma endregion

		if (ViewModes == DEPTH)
		{
#pragma region  DEBUG_DEPTH

			//Depth debug
			glViewport(0, 0, GetWidth(), GetHeight());

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(depthDebugProgram);
			glUniformMatrix4fv(glGetUniformLocation(depthDebugProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(glGetUniformLocation(depthDebugProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

			model->Draw(depthDebugProgram);
			glActiveTexture(GL_TEXTURE0); // TODO: удалить

#pragma endregion
		}
		else
		{
#pragma region LIGHT_CULLING_COMPUTE
			{
				glDepthFunc(GL_EQUAL);
				glClear(GL_COLOR_BUFFER_BIT);

				workGroupsX = (GetWidth() + (GetWidth() % TILE_SIZE)) / TILE_SIZE;
				workGroupsY = (GetHeight() + (GetHeight() % TILE_SIZE)) / TILE_SIZE;

				glUseProgram(lightCullingProgram);

				glUniformMatrix4fv(glGetUniformLocation(lightCullingProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				glUniformMatrix4fv(glGetUniformLocation(lightCullingProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

				// Bind shader storage buffer objects for the light and indice buffers
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBuffer);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indexBuffer);

				// Bind depth map texture to texture location 4 (which will not be used by any model texture)
				glActiveTexture(GL_TEXTURE4);
				glUniform1i(glGetUniformLocation(lightCullingProgram, "depthMap"), 4);
				glBindTexture(GL_TEXTURE_2D, depthMap);

				// Dispatch the compute shader, using the workgroup values calculated earlier
				glDispatchCompute(workGroupsX, workGroupsY, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

				// Unbind the depth map
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
#pragma endregion

#pragma region RENDER_FBO
			{
				glDepthFunc(GL_LESS);

				glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glUseProgram(lightProgram);
				glUniform3fv(glGetUniformLocation(lightProgram, "viewPosition"), 1, glm::value_ptr(camera.Position));
				glUniformMatrix4fv(glGetUniformLocation(lightProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				glUniformMatrix4fv(glGetUniformLocation(lightProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(lightProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

				if (ViewModes == LIGHT)
				{
					glUniform1i(glGetUniformLocation(lightProgram, "doLightDebug"), 1);
				}
				else glUniform1i(glGetUniformLocation(lightProgram, "doLightDebug"), 0);

				model->Draw(lightProgram);
				glActiveTexture(GL_TEXTURE0); // TODO: удалить

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
#pragma endregion

#pragma region RENDER_QUAD
			{
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Weirdly, moving this call drops performance into the floor
				glUseProgram(renderProgram);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, colorBuffer);

				//Quad Render // TODO: переделать
				{
					static GLuint      mQuadVAO = 0;
					static GLuint      mQuadVBO = 0;
					if (mQuadVAO == 0)
					{
						GLfloat quadVertices[] = {
							-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
							-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
							1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
							1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
						};

						glGenVertexArrays(1, &mQuadVAO);
						glGenBuffers(1, &mQuadVBO);
						glBindVertexArray(mQuadVAO);
						glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
						glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
						glEnableVertexAttribArray(0);
						glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
						glEnableVertexAttribArray(1);
						glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
					}

					glBindVertexArray(mQuadVAO);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					glBindVertexArray(0);
				}

				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
			}
#pragma endregion
		}

		glDisable(GL_DEPTH_TEST);
	}
}
//=============================================================================
void TestForwardPlus::OnImGuiDraw()
{
	// Create a simple ImGUI config window.
	{
		ImGui::Begin("Forward+ Shading");

		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char*)glGetString(GL_VENDOR));
		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char*)glGetString(GL_VERSION));
		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char*)glGetString(GL_RENDERER));
		ImGui::Separator();

		ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::Separator();

		ImGui::Text("Tile size: %ix%i", TILE_SIZE, TILE_SIZE);
		ImGui::Text("Max lights per tile: %i", MAX_LIGHTS_PER_TILE);

		if (ImGui::CollapsingHeader("View Mode"))
		{

			const char* items[] = { "Shaded", "Depth", "Light Debug" };
			static int item_current = 0;
			ImGui::Combo("Mode", &item_current, items, IM_ARRAYSIZE(items));

			ViewModes = static_cast<Modes>(item_current);

		}

		// here i have some bugs when debugging.
		// prob this is not the proper way to change num of lights dynamically...
		if (ImGui::SliderInt("Lights count", &numberOfLights, 1, 130))
		{
			SetupLights();
			glUseProgram(lightCullingProgram);
			glUniform1i(glGetUniformLocation(lightCullingProgram, "lightCount"), numberOfLights);
		}

		if (ImGui::Button("Recalculate lights"))
		{
			UpdateLights();
			SetupLights();
			glUseProgram(lightCullingProgram);
			glUniform1i(glGetUniformLocation(lightCullingProgram, "lightCount"), numberOfLights);
		}
		//ImGui::ListBox

		ImGui::End();
	}
}
//=============================================================================
void TestForwardPlus::OnResize(uint16_t width, uint16_t height)
{
}
//=============================================================================