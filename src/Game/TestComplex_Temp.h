#pragma once

#include <map>
#include <fstream>
#include <iostream>

#pragma region Scene

struct Object
{
    virtual void addChildren(std::shared_ptr<Object> obj, const glm::mat4& offset = glm::mat4(1.0f));

    std::vector<std::pair<std::shared_ptr<Object>, glm::mat4>> children;
};

struct PointLight : public Object
{
    glm::vec3 intensity;
    glm::vec3 position;
    float width;

    PointLight(const glm::vec3& i, const glm::vec3& p, float w = 0.1f);
};

struct Scene
{
    std::shared_ptr<Object> root;
    glm::vec3 ambient_light_irradiance;

    Scene();

    void add(std::shared_ptr<Object> obj, const glm::mat4& offset = glm::mat4(1.0f));
};

struct Camera2
{
    glm::vec3 cam_pos;
    glm::vec3 cam_dir;
    glm::vec3 cam_up;

    float pitch = 0.0f, yaw = -90.0f;

    float fov = 60.0f;
    float aspect = 1.0f;
    float near = 0.1f;
    float far = 100.0f;

    glm::mat4 view;
    glm::mat4 projection;

    Camera2();

    void eval();
};

struct CameraControl
{
    Camera2* camera = nullptr;

    void bind(Camera2* c);

    void onEvents();
};


#pragma endregion

#pragma region Texture

class Texture
{
protected:
    GLuint handle_;

public:
    virtual void use(int unit_id) = 0;
    virtual GLuint id() const = 0;
};

class Texture2D : public Texture
{
    std::string filename_;

    void genTexture();
    void setDefaultParams();

public:
    Texture2D(const std::string& tex_name = "");
    Texture2D(int width, int height, const void* data, GLuint intformat = GL_RGB, GLuint format = GL_RGB, GLuint dtype = GL_RGB);
    Texture2D(int width, int height);
    void setParami(GLuint k, GLuint v);
    virtual void use(int unit_id) override;
    virtual GLuint id() const override;
};

class TextureCube : public Texture
{
    void genTexture();
    void setDefaultParams();

public:
    TextureCube(int width, int height, GLuint intformat = GL_RGB, GLuint format = GL_RGB, GLuint dtype = GL_RGB);
    void setParami(GLuint k, GLuint v);
    virtual void use(int unit_id) override;
    virtual GLuint id() const override;
};

#pragma endregion

#pragma region Shader

class Shader
{
    int sp_;
    static GLuint loadShader(const std::string& vs_name, const std::string& fs_name);

    std::map<std::string, Texture*> textures;

public:
    Shader(const std::string& vs_name, const std::string& fs_name);
    void use();
    void setUniformi(const std::string& name, int x);
    void setUniform(const std::string& name, float x);
    void setUniform(const std::string& name, float x, float y, float z);
    void setUniform(const std::string& name, const glm::vec3& value);
    void setUniform(const std::string& name, const glm::mat4& value);
    void setUniblock(const std::string& name, int idx);
    void setLights(const std::vector<PointLight>& lights);
    void setMVP(const glm::mat4& view, const glm::mat4& projection);
    void setCamera(const Camera2& camera);
    void setTexture(const std::string& name, Texture* texture);
    void solveTextures();
    GLuint id();
};


#pragma endregion

#pragma region UniformBuffer

class UniformBuffer
{
    GLuint ubo;

public:
    UniformBuffer();
    void use(int id);
    void setData(int size, const void* value, GLuint mode);
};

#pragma endregion

#pragma region FramebufferObject

class FramebufferObject
{
    GLuint rbo_;
    int width_;
    int height_;
    std::vector<GLuint> atts;

public:
    GLuint fb_;
    FramebufferObject(const std::vector<Texture2D*>& attachments, int width, int height, const Texture2D* depth_attachments = nullptr);
    FramebufferObject(const std::vector<TextureCube*>& attachments, int width, int height, int cube_idx, const TextureCube* depth_attachments = nullptr);

    void use();
};

#pragma endregion

#pragma region Material

class Material
{
    std::map<std::string, Texture2D*> textures;
    std::map<std::string, int> properties_i;
    std::map<std::string, float> properties_f;
    std::map<std::string, glm::vec3> properties_v3;

    static std::map<std::string, Texture2D*> texture_pool;

    void loadTexturesAssimpType(aiTextureType type, const std::string& type_name, aiMaterial* mat, const std::string& dir = "");
    void loadTexturesAssimp(aiMaterial* mat, const std::string& dir = "");

public:
    Material();

    Material(aiMaterial* mat, const std::string& dir = "");

    void load(aiMaterial* mat, const std::string& dir = "");

    void use(Shader& shader);
};

#pragma endregion

#pragma region Mesh

class Mesh2
{
    std::vector<GLfloat> vertices_;
    std::vector<GLuint> indices_;
    Material material_;

    GLuint vao_;
    GLuint vbo_;
    GLuint ebo_;
    int n_;

    void loadMeshAssimp(aiMesh* mesh, const aiScene* scene, const std::string& dir);

public:
    Mesh2();

    Mesh2(aiMesh* mesh, const aiScene* scene, const std::string& dir = "");

    void draw(Shader& shader);
};

#pragma endregion

#pragma region Model

class Model2 : public Object
{
    std::vector<Mesh2> meshes;
    void loadModelAssimp(const std::string& filename);

public:
    std::vector<std::pair<std::shared_ptr<Model2>, glm::mat4>> children;
    Model2();

    Model2(const std::string& filename);
    void addChildren(std::shared_ptr<Model2> model, const glm::mat4& offset = glm::mat4(1.0f));
    void draw(Shader& shader, const glm::mat4& model_matrix = glm::mat4(1.0f));
};

#pragma endregion

#pragma region SceneDesc

struct SceneDesc
{
    Model2 models;
    glm::vec3 ambient_light_irradiance;
    std::vector<PointLight> point_lights;

    SceneDesc();

    void load(std::shared_ptr<Object> obj);
    SceneDesc(std::shared_ptr<Object> obj);
    SceneDesc(Scene& obj);
};


#pragma endregion

#pragma region ShadowMapper

struct ShadowMapper
{
    const GLuint SHADOW_MAP_WIDTH = 512, SHADOW_MAP_HEIGHT = 512;
    Shader shadow_shader;
    TextureCube depth_texture;
    TextureCube pos_texture;
    TextureCube normal_texture;
    TextureCube flux_texture;  // out flux
    FramebufferObject shadow_map_fbo[6];
    float shadow_limit = 100.0;

    ShadowMapper();
    void lightPass(glm::vec3 shadow_light_pos, glm::vec3 shadow_light_int, Model2* scene);
    void attach(Shader& shader);
};
#pragma endregion

#pragma region Deferred

struct Deferred
{
    static const int n_gbuffer = 8;
    std::vector<GLfloat> vertices;
    GLuint vao, vbo;
    Shader gbuffer_shader;
    Texture2D gbuffer_texture[n_gbuffer];
    FramebufferObject gbuffer_fbo;
    int width_, height_;

    Deferred(int width, int height);
    void drawGeometry(Model2* scene, const Camera2& camera);
    void drawLighting(Shader& lighting_shader);
};

#pragma endregion
