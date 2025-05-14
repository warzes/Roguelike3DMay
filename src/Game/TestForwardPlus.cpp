#include "stdafx.h"
#include "TestForwardPlus.h"
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
	mat4 model;
	mat4 view;
	mat4 projection;
};

void main()
{
	vsOut.FragPos = mat3(model) * aVertexPosition;
	vsOut.Normal = transpose(inverse(mat3(model))) * aVertexNormal;
	vsOut.TexCoords = aVertexTexCoords;
	gl_Position = projection * view * model * vec4(aVertexPosition, 1.0);
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

layout(std140, binding = 1) uniform LightData {
	vec3 lightPos;
	vec3 viewPos;
};

layout(binding = 0) uniform sampler2D diffuseTexture1;

out vec4 FragColorOut;

// Blinn-Phong with glTF model
void main()
{
	vec3 color = texture(diffuseTexture1, fsIn.TexCoords).rgb;

	// ambient
	vec3 ambient = 0.05 * color;

	// diffuse
	vec3 lightDir = normalize(lightPos - fsIn.FragPos);
	vec3 normal = normalize(fsIn.Normal);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * color;

	// specular
	vec3 viewDir = normalize(viewPos - fsIn.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;

	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	vec3 specular = vec3(0.3) * spec; // assuming bright white light color
	FragColorOut = vec4(ambient + diffuse + specular, 1.0);
}
)";

	const char* lightShaderCodeVertex = R"(
#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";
	const char* lightShaderCodeFragment = R"(
#version 460 core
out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 0.8, 0.2, 1.0);
}
)";
	
	struct alignas(16) MVPData final
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
	};

	struct alignas(16) Light2Data final
	{
		glm::vec3 lightPos;
		glm::vec3 viewPos;
	};

	GLuint program;

	MVPData mvpData;
	GLuint mvpUbo;

	Light2Data lightData;
	GLuint lightUbo;

	GLuint lightDrawProgram;
	int lightModelLoc;
	int lightViewLoc;
	int lightProjLoc;

	GLuint texture;
	GLuint vbo;
	GLuint vao;

	Camera camera;

	glm::vec3 lightPos(1.0f, 0.5f, 0.0f);

	Model* model;

#pragma region Forward+
	const char* depthShaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 aVertexPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aVertexPosition, 1.0);
}
)";

	const char* depthShaderCodeFragment = R"(
#version 460 core

void main()
{
}
)";

	const char* depthDebugShaderCodeVertex = R"(
#version 460 core

layout (location = 0) in vec3 aVertexPosition;
layout (location = 1) in vec3 aVertexNormal;
layout (location = 2) in vec2 aVertexTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;

void main()
{
	gl_Position = projection * view * model * vec4(aVertexPosition, 1.0);
	TexCoords = aVertexTexCoords;
}
)";

	const char* depthDebugShaderCodeFragment = R"(
#version 460 core

uniform float near;
uniform float far;

out vec4 fragColor;

// Need to linearize the depth because we are using the projection
float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
	float depth = LinearizeDepth(gl_FragCoord.z) / far;
	fragColor = vec4(vec3(depth), 1.0);
}
)";

	const char* lightCullingShaderCodeCompute = R"(
#version 460 core

#define MAX_LIGHTS_PER_TILE 128
#define TILE_SIZE 16

struct PointLight {
	vec4 position;
	vec4 color;
	vec4 paddingAndRadius;
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

// Shared values between all the threads in the group
shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visibleLightCount;
shared vec4 frustumPlanes[6];
// Shared local storage for visible indices, will be written out to the global buffer at the end
shared int visibleLightIndices[MAX_LIGHTS_PER_TILE];
//shared mat4 viewProjection;


layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE) in;
void main() {

	ivec2 tile_id = ivec2(gl_WorkGroupID.xy);

	if (gl_LocalInvocationIndex == 0) {
		minDepthInt = 0xFFFFFFFF;
		maxDepthInt = 0x0;
		visibleLightCount = 0;
		uint index = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
		uint offset = index * MAX_LIGHTS_PER_TILE;
		for (uint i = 0; i < MAX_LIGHTS_PER_TILE; i++) {
			lights_indices[offset + i] = -1;
		}
	}
	barrier();

	// Compute depth min and max of the workgroup
	vec2 screen_uv = gl_GlobalInvocationID.xy / screenSize;

	float depth = texture(depthMap, screen_uv).r;

	uint depth_uint = floatBitsToUint(depth);
	atomicMin(minDepthInt, depth_uint);
	atomicMax(minDepthInt, depth_uint);

	barrier();

	// Compute Tile frustrum planes
	if (gl_LocalInvocationIndex == 0) {
		float min_group_depth = uintBitsToFloat(minDepthInt);
		float max_group_depth = uintBitsToFloat(maxDepthInt);

		vec4 vs_min_depth = (inverse(projection) * vec4(0.0, 0.0, (2.0 * min_group_depth - 1.0), 1.0));
		vec4 vs_max_depth = (inverse(projection) * vec4(0.0, 0.0, (2.0 * max_group_depth - 1.0), 1.0));
		vs_min_depth /= vs_min_depth.w;
		vs_max_depth /= vs_max_depth.w;
		min_group_depth = vs_min_depth.z;
		max_group_depth = vs_max_depth.z;

		vec2 tile_scale = vec2(screenSize) * (1.0 / float(2 * TILE_SIZE));
		vec2 tile_bias = tile_scale - vec2(gl_WorkGroupID.xy);

		vec4 col1 = vec4(-projection[0][0] * tile_scale.x, projection[0][1], tile_bias.x, projection[0][3]);
		vec4 col2 = vec4(projection[1][0], -projection[1][1] * tile_scale.y, tile_bias.y, projection[1][3]);
		vec4 col4 = vec4(projection[3][0], projection[3][1], -1.0, projection[3][3]);

		frustumPlanes[0] = col4 + col1;
		frustumPlanes[1] = col4 - col1;
		frustumPlanes[2] = col4 - col2;
		frustumPlanes[3] = col4 + col2;
		frustumPlanes[4] = vec4(0.0, 0.0, 1.0, -min_group_depth);
		frustumPlanes[5] = vec4(0.0, 0.0, -1.0, max_group_depth);
		for (uint i = 0; i < 4; i++) {
			frustumPlanes[i] *= 1.0f / length(frustumPlanes[i].xyz);
		}
	}

	barrier();

	// Cull lights
	uint thread_count = TILE_SIZE * TILE_SIZE;
	for (uint i = gl_LocalInvocationIndex; i < lightCount; i += thread_count) {
		PointLight light = data[i];
		vec4 vs_light_pos = view * vec4(light.position);

		if (visibleLightCount < MAX_LIGHTS_PER_TILE) {
			bool inFrustum = true;
			for (uint j = 0; j < 6 && inFrustum; j++) {
				float d = dot(frustumPlanes[j], vs_light_pos);
				inFrustum = (d >= -light.paddingAndRadius.w);
			}
			if (inFrustum) {
				uint id = atomicAdd(visibleLightCount, 1);
				visibleLightIndices[id] = int(i);
			}
		}
	}

	barrier();
	if (gl_LocalInvocationIndex == 0) {
		uint index = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
		uint offset = index * MAX_LIGHTS_PER_TILE;
		for (uint i = 0; i < visibleLightCount; i++) {
			lights_indices[offset + i] = visibleLightIndices[i];
		}
	}
}
)";

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

in VERTEX_OUT{
    vec2 frag_uv;
    mat3 TBN;
    vec3 ts_frag_pos;
    vec3 ts_view_pos;
} fragment_in;

struct PointLight {
	vec4 position;
	vec4 color;
	vec4 paddingAndRadius;
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
	ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileID = location / ivec2(16, 16);
	uint index = tileID.y * numberOfTilesX + tileID.x;
    uint offset = index * MAX_LIGHTS_PER_TILE;

    vec4 result = vec4(0.0, 0.0, 0.0, 1.0);

	vec4 base_diffuse = texture(texture_diffuse1, fragment_in.frag_uv);

	vec3 normal = texture(texture_normal1, fragment_in.frag_uv).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	float specpower = 60.0f;

	for (uint i = 0; i < MAX_LIGHTS_PER_TILE; i++)
	{
		if (lights_indices[offset + i] != -1) 
		{
		    int indices = lights_indices[offset + i];

			PointLight light = lightBuffer.data[indices];

			vec3 ts_light_pos = fragment_in.TBN * vec3(light.position);
			vec3 ts_light_dir = normalize(ts_light_pos - fragment_in.ts_frag_pos);
			float dist = length(ts_light_pos - fragment_in.ts_frag_pos);
	
			vec3 N = normal;
			vec3 L = ts_light_dir;
	
			vec3 R = reflect(-L, N);
			float NdotR = max(0.0, dot(N, R));
			float NdotL = max(0.0, dot(N, L));
	
			float attenuation = clamp(1.0 - dist * dist / (light.paddingAndRadius.w * light.paddingAndRadius.w), 0.0, 1.0);
	
			vec3 diffuse_color  = 1.0 * vec3(light.color.x, light.color.y, light.color.z) * vec3(base_diffuse.r, base_diffuse.g, base_diffuse.b) * NdotL * attenuation;
			vec3 specular_color = vec3(1.0) * pow(NdotR, specpower) * attenuation;
	
			result += vec4(diffuse_color + specular_color, 0.0);	
		}
	}

	if (base_diffuse.a <= 0.2) {
		discard;
	}
	
	fragColor = result;
	
	if (doLightDebug==1){
		uint count;
		for (uint i = 0; i < MAX_LIGHTS_PER_TILE; i++) {
		    if (lights_indices[offset + i] != -1 ) {
			count++;
		    }
		}
		float shade = float(count) / float(MAX_LIGHTS_PER_TILE * 2); 
		fragColor = vec4(shade, shade, shade, 1.0);
	}
}
)";

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

	struct alignas(16) LightData final
	{
		glm::vec4 position;
		glm::vec4 color;
		glm::vec4 paddingAndRadius;
	};

	GLuint depthProgram;
	GLuint depthDebugProgram;
	GLuint lightCullingProgram;
	GLuint lightingProgram;
	GLuint finalProgram;

	int numberOfLights{ 35 };

	GLuint lightBufferSSBO;
	GLuint lightIndexBufferSSBO;

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
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfLights * sizeof(Light), NULL, GL_DYNAMIC_DRAW);

		if (lightBufferSSBO == 0) return;

		//glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfLights * sizeof(LightData), NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBufferSSBO);
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

			light.paddingAndRadius = { 0.0f, 0.0f, 0.0f, 8.0f };
		}

		//glUnmapNamedBuffer(lightBufferSSBO);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void UpdateLights()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBufferSSBO);

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
	EngineConfig config{};
	config.render.vsync = true;

	return config;
}
//=============================================================================
bool TestForwardPlus::OnCreate()
{
	program = gl4::CreateShaderProgram(shaderCodeVertex, shaderCodeFragment);

	mvpUbo = gl4::CreateBuffer(GL_DYNAMIC_STORAGE_BIT, sizeof(MVPData), nullptr);
	lightUbo = gl4::CreateBuffer(GL_DYNAMIC_STORAGE_BIT, sizeof(Light2Data), nullptr);

	lightDrawProgram = gl4::CreateShaderProgram(lightShaderCodeVertex, lightShaderCodeFragment);
	lightModelLoc = gl4::GetUniformLocation(lightDrawProgram, "model");
	lightViewLoc = gl4::GetUniformLocation(lightDrawProgram, "view");
	lightProjLoc = gl4::GetUniformLocation(lightDrawProgram, "projection");

	texture = gl4::LoadTexture2D("data/textures/wood.png", false);

	model = new Model("data/mesh/Sponza/Sponza.gltf");

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

	vbo = gl4::CreateBuffer(0, sizeof(vertices), vertices);
	vao = gl4::CreateVertexArray(vbo, sizeof(Vertex), attribs);

	camera.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));

#pragma region Forward+

	ViewModes = Modes::SHADED;

	workGroupsX = (GetWidth() + (GetWidth() % TILE_SIZE)) / TILE_SIZE;
	workGroupsY = (GetHeight() + (GetHeight() % TILE_SIZE)) / TILE_SIZE;

	GLuint tilesCount = workGroupsX * workGroupsY;

	depthProgram = gl4::CreateShaderProgram(depthShaderCodeVertex, depthShaderCodeFragment);
	depthDebugProgram = gl4::CreateShaderProgram(depthDebugShaderCodeVertex, depthDebugShaderCodeFragment);
	lightCullingProgram = gl4::CreateShaderProgram(lightCullingShaderCodeCompute);
	lightingProgram = gl4::CreateShaderProgram(lightingShaderCodeVertex, lightingShaderCodeFragment);
	finalProgram = gl4::CreateShaderProgram(finalShaderCodeVertex, finalShaderCodeFragment);

	glUseProgram(depthProgram);
	glUniformMatrix4fv(glGetUniformLocation(depthProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

	glUseProgram(depthDebugProgram);
	glUniformMatrix4fv(glGetUniformLocation(depthDebugProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	glUniform1f(glGetUniformLocation(depthDebugProgram, "near"), 0.01f);
	glUniform1f(glGetUniformLocation(depthDebugProgram, "far"), 1000.0f);


	glUseProgram(lightCullingProgram);
	glUniform1i(glGetUniformLocation(lightCullingProgram, "lightCount"), numberOfLights);
	glUniform2fv(glGetUniformLocation(lightCullingProgram, "screenSize"), 1, glm::value_ptr(glm::vec2(GetWidth(), GetHeight())));


	glUseProgram(lightingProgram);
	glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	glUniform1i(glGetUniformLocation(lightingProgram, "numberOfTilesX"), workGroupsX);
	glUniform1i(glGetUniformLocation(lightingProgram, "doLightDebug"), 0);
	glUniform3fv(glGetUniformLocation(lightingProgram, "viewPosition"), 1, glm::value_ptr(camera.Position));

	// create light buffer
	//lightBufferSSBO = gl4::CreateBuffer(GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, numberOfLight * sizeof(LightData), nullptr);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBufferSSBO);
	//// create visible light indices buffer
	//lightIndexBufferSSBO = gl4::CreateBuffer(GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, sizeof(int) * tilesCount * MAX_LIGHTS_PER_TILE, nullptr);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightIndexBufferSSBO);

	// Create light buffer
	glGenBuffers(1, &lightBufferSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBufferSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfLights * sizeof(LightData), 0, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBufferSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	// Create visible light indices buffer
	glGenBuffers(1, &lightIndexBufferSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexBufferSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int)* workGroupsX* workGroupsY* MAX_LIGHTS_PER_TILE, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightIndexBufferSSBO);
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

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	//glEnable(GL_CULL_FACE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}
//=============================================================================
void TestForwardPlus::OnDestroy()
{
	glDeleteTextures(1, &texture);
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
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
	//gl4::SetFrameBuffer(0, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//mvpData.model = glm::mat4(1.0f);
	mvpData.view = camera.GetViewMatrix();
	mvpData.projection = glm::perspective(glm::radians(60.0f), GetAspect(), 0.01f, 1000.0f);

	//lightData.lightPos = lightPos;
	//lightData.viewPos = camera.Position;

	//glNamedBufferSubData(mvpUbo, 0, sizeof(MVPData), &mvpData);
	//glNamedBufferSubData(lightUbo, 0, sizeof(Light2Data), &lightData);

	//// вывод квада плоскости
	//{
	//	glUseProgram(program);
	//	glBindBufferBase(GL_UNIFORM_BUFFER, 0, mvpUbo);
	//	glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightUbo);
	//	glBindTextureUnit(0, texture);
	//	glBindVertexArray(vao);

	//	glDrawArrays(GL_TRIANGLES, 0, 6);
	//}

	// вывод модели
	//{
	//	glUseProgram(program);
	//	glBindBufferBase(GL_UNIFORM_BUFFER, 0, mvpUbo);
	//	glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightUbo);
	//	model->Draw(program);
	//}

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

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexBufferSSBO);
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

			////update depth uniforms
			//glViewport(0, 0, GetWidth(), GetHeight());

			glEnable(GL_POLYGON_OFFSET_FILL);
			//glPolygonOffset(4.0f, 4.0f);

			glUseProgram(depthProgram);
			static const GLenum buffs[] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, buffs);
			glClearBufferfv(GL_COLOR, 0, float_zeros);
			glClearBufferfv(GL_DEPTH, 0, float_ones);

			glUniformMatrix4fv(glGetUniformLocation(depthProgram, "projection"), 1, GL_FALSE, glm::value_ptr(mvpData.projection));
			glUniformMatrix4fv(glGetUniformLocation(depthProgram, "view"), 1, GL_FALSE, glm::value_ptr(mvpData.view));
			glUniformMatrix4fv(glGetUniformLocation(depthProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

			model->Draw(depthProgram);

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
			glUniformMatrix4fv(glGetUniformLocation(depthDebugProgram, "projection"), 1, GL_FALSE, glm::value_ptr(mvpData.projection));
			glUniformMatrix4fv(glGetUniformLocation(depthDebugProgram, "view"), 1, GL_FALSE, glm::value_ptr(mvpData.view));

			model->Draw(depthDebugProgram);

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

				glUniformMatrix4fv(glGetUniformLocation(lightCullingProgram, "projection"), 1, GL_FALSE, glm::value_ptr(mvpData.projection));
				glUniformMatrix4fv(glGetUniformLocation(lightCullingProgram, "view"), 1, GL_FALSE, glm::value_ptr(mvpData.view));

				// Bind shader storage buffer objects for the light and indice buffers
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBufferSSBO);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightIndexBufferSSBO);

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

				glUseProgram(lightingProgram);
				glUniform3fv(glGetUniformLocation(lightingProgram, "viewPosition"), 1, glm::value_ptr(camera.Position));
				glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "projection"), 1, GL_FALSE, glm::value_ptr(mvpData.projection));
				glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "view"), 1, GL_FALSE, glm::value_ptr(mvpData.view));
				glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

				if (ViewModes == LIGHT)
				{
					glUniform1i(glGetUniformLocation(lightingProgram, "doLightDebug"), 1);
				}
				else glUniform1i(glGetUniformLocation(lightingProgram, "doLightDebug"), 0);

				model->Draw(lightingProgram);

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
#pragma endregion

#pragma region RENDER_QUAD
			{
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Weirdly, moving this call drops performance into the floor
				glUseProgram(finalProgram);
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

	//// рендер источника света
	//{
	//	glUseProgram(lightDrawProgram);
	//	gl4::SetUniform(lightProjLoc, mvpData.projection);
	//	gl4::SetUniform(lightViewLoc, mvpData.view);

	//	glm::mat4 modelMat = glm::mat4(1.0f);
	//	modelMat = glm::translate(modelMat, lightPos);
	//	modelMat = glm::scale(modelMat, glm::vec3(0.2f));
	//	gl4::SetUniform(lightModelLoc, modelMat);

	//	GetGraphicSystem().DrawCube();
	//}
}
//=============================================================================
void TestForwardPlus::OnImGuiDraw()
{
	// Create a simple ImGUI config window.
	{
		ImGui::Begin("SweetGL - Forward+ Shading");

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