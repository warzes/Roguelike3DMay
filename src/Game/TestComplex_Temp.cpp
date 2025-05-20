#include "stdafx.h"
#include "TestComplex_Temp.h"

#pragma region Scene

void Object::addChildren(std::shared_ptr<Object> obj, const glm::mat4& offset)
{
    children.push_back({ obj, offset });
}

PointLight::PointLight(const glm::vec3& i, const glm::vec3& p, float w) : intensity(i), position(p), width(w) {}

Scene::Scene() : root(std::make_shared<Object>())
{
}

void Scene::add(std::shared_ptr<Object> obj, const glm::mat4& offset)
{
    root->addChildren(obj, offset);
}

Camera2::Camera2()
{
    cam_pos.x = 0;
    cam_pos.y = 3;
    cam_pos.z = 0;
    cam_dir.z = -1;
    cam_up.y = 1;
    eval();
}

void Camera2::eval()
{
    view = glm::lookAt(cam_pos, cam_pos + cam_dir, cam_up);
    projection = glm::perspective(glm::radians(fov), aspect, near, far);
}

void CameraControl::bind(Camera2* c)
{
    camera = c;
}

void CameraControl::onEvents()
{
    if (camera == nullptr)
    {
        return;
    }

    glm::vec3 cam_pos = camera->cam_pos;
    glm::vec3 cam_dir = camera->cam_dir;
    glm::vec3 cam_up = camera->cam_up;

    float pitch = camera->pitch;
    float yaw = camera->yaw;

    static GLfloat last_frame_time = glfwGetTime();
    static GLfloat last_cursor_x = GetMousePositionX(), last_cursor_y = GetMousePositionY();

    GLfloat current_time = glfwGetTime();

    GLfloat delta_cursor_x = GetMousePositionX() - last_cursor_x;
    GLfloat delta_cursor_y = GetMousePositionY() - last_cursor_y;

    last_cursor_x = GetMousePositionX();
    last_cursor_y = GetMousePositionY();

    GLfloat cam_speed = 1.0f;
    GLfloat cam_pan_speed = 0.005f;
    GLfloat cam_rotate_speed = 0.2f;

    if (GetKeyDown(GLFW_KEY_LEFT_ALT) && GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
    {
        pitch += -89.0f * delta_cursor_y * cam_rotate_speed / 512;
        yaw += 1080.0f * delta_cursor_x * cam_rotate_speed / 512;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    cam_dir.x = glm::cos(glm::radians(pitch)) * glm::cos(glm::radians(yaw));
    cam_dir.y = glm::sin(glm::radians(pitch));
    cam_dir.z = glm::cos(glm::radians(pitch)) * glm::sin(glm::radians(yaw));
    cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cam_hand = glm::cross(cam_dir, cam_up);
    cam_up = glm::cross(cam_hand, cam_dir);
    cam_up = glm::normalize(cam_up);
    cam_hand = glm::cross(cam_dir, cam_up);

    if (GetKeyDown(GLFW_KEY_LEFT_CONTROL) && GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
    {
        cam_pos += cam_hand * delta_cursor_x * cam_pan_speed;
        cam_pos -= cam_up * delta_cursor_y * cam_pan_speed;
    }

    if (GetKeyDown(GLFW_KEY_LEFT_SHIFT) && GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
    {
        cam_pos += cam_hand * delta_cursor_x * cam_pan_speed;
        cam_pos -= cam_dir * delta_cursor_y * cam_pan_speed;
    }

    if (GetKeyDown(GLFW_KEY_UP))
    {
        cam_pos += cam_dir * cam_speed * (current_time - last_frame_time);
    }
    if (GetKeyDown(GLFW_KEY_DOWN))
    {
        cam_pos -= cam_dir * cam_speed * (current_time - last_frame_time);
    }
    if (GetKeyDown(GLFW_KEY_LEFT))
    {
        cam_pos -= cam_hand * cam_speed * (current_time - last_frame_time);
    }
    if (GetKeyDown(GLFW_KEY_RIGHT))
    {
        cam_pos += cam_hand * cam_speed * (current_time - last_frame_time);
    }
    last_frame_time = current_time;

    camera->cam_pos = cam_pos;
    camera->cam_dir = cam_dir;
    camera->cam_up = cam_up;

    camera->yaw = yaw;
    camera->pitch = pitch;

    camera->eval();
}

#pragma endregion

#pragma region Texture

void Texture2D::setDefaultParams()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture2D::Texture2D(const std::string& tex_name)
{
    int sx, sy;

    glGenTextures(1, &handle_);
    glBindTexture(GL_TEXTURE_2D, handle_);
    setDefaultParams();

    if (tex_name != "")
    {
        unsigned char* tex_data = stbi_load(tex_name.c_str(), &sx, &sy, nullptr, 3);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sx, sy, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
    }
    filename_ = tex_name;
}

Texture2D::Texture2D(int width, int height, const void* data, GLuint intformat, GLuint format, GLuint dtype)
{
    glGenTextures(1, &handle_);
    glBindTexture(GL_TEXTURE_2D, handle_);
    setDefaultParams();

    glTexImage2D(GL_TEXTURE_2D, 0, intformat, width, height, 0, format, dtype, data);
}

Texture2D::Texture2D(int width, int height) : Texture2D(width, height, nullptr, GL_RGBA16F, GL_RGBA, GL_FLOAT)
{
}

void Texture2D::setParami(GLuint k, GLuint v)
{
    glBindTexture(GL_TEXTURE_2D, handle_);
    glTexParameteri(GL_TEXTURE_2D, k, v);
}

void Texture2D::use(int unit_id)
{
    glActiveTexture(GL_TEXTURE0 + unit_id);
    glBindTexture(GL_TEXTURE_2D, handle_);
}

GLuint Texture2D::id() const
{
    return handle_;
}

void TextureCube::setDefaultParams()
{
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
   // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

TextureCube::TextureCube(int width, int height, GLuint intformat, GLuint format, GLuint dtype)
{
    glGenTextures(1, &handle_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, handle_);
    setDefaultParams();
    for (int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, intformat, width, height, 0, format, dtype, nullptr);
}

void TextureCube::setParami(GLuint k, GLuint v)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, handle_);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, k, v);
}

void TextureCube::use(int unit_id)
{
    glActiveTexture(GL_TEXTURE0 + unit_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, handle_);
}

GLuint TextureCube::id() const
{
    return handle_;
}

#pragma endregion

#pragma region Shader

GLuint Shader::loadShader(const std::string& vs_name, const std::string& fs_name)
{
    std::ifstream vs_ifs(vs_name), fs_ifs(fs_name);
    std::stringstream vs_ss, fs_ss;
    vs_ss << vs_ifs.rdbuf();
    fs_ss << fs_ifs.rdbuf();
    std::string vs_src = vs_ss.str(), fs_src = fs_ss.str();
    const GLchar* vs_src_ptr = vs_src.c_str();
    const GLchar* fs_src_ptr = fs_src.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src_ptr, nullptr);
    glCompileShader(vs);

    GLint vs_stat;
    char vs_info[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &vs_stat);
    if (!vs_stat)
    {
        glGetShaderInfoLog(vs, 512, nullptr, vs_info);
        std::cerr << "VS Compiler error: " << vs_name << ": " << vs_info << std::endl;
        return 0;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src_ptr, nullptr);
    glCompileShader(fs);

    GLint fs_stat;
    char fs_info[512];
    glGetShaderiv(fs, GL_COMPILE_STATUS, &fs_stat);
    if (!fs_stat)
    {
        glGetShaderInfoLog(fs, 512, nullptr, fs_info);
        std::cerr << "FS Compiler error: " << fs_name << ": " << fs_info << std::endl;
        return 0;
    }

    GLuint sp = glCreateProgram();
    glAttachShader(sp, vs);
    glAttachShader(sp, fs);
    glLinkProgram(sp);

    GLint sp_stat;
    char sp_info[512];
    glGetProgramiv(sp, GL_LINK_STATUS, &sp_stat);
    if (!sp_stat)
    {
        glGetProgramInfoLog(sp, 512, nullptr, sp_info);
        std::cerr << "Shader Link error: " << vs_name << ": " << sp_info << std::endl;
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return sp;
}

Shader::Shader(const std::string& vs_name, const std::string& fs_name)
{
    sp_ = loadShader(vs_name, fs_name);
}

void Shader::use()
{
    glUseProgram(sp_);
}

void Shader::setUniformi(const std::string& name, int x)
{
    glUniform1i(glGetUniformLocation(sp_, name.c_str()), x);
}

void Shader::setUniform(const std::string& name, float x)
{
    glUniform1f(glGetUniformLocation(sp_, name.c_str()), x);
}

void Shader::setUniform(const std::string& name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(sp_, name.c_str()), x, y, z);
}

void Shader::setUniform(const std::string& name, const glm::vec3& value)
{
    glUniform3fv(glGetUniformLocation(sp_, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string& name, const glm::mat4& value)
{
    glUniformMatrix4fv(glGetUniformLocation(sp_, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniblock(const std::string& name, int idx)
{
    glUniformBlockBinding(sp_, glGetUniformBlockIndex(sp_, name.c_str()), idx);
}

void Shader::setLights(const std::vector<PointLight>& lights)
{
    setUniformi("n_point_light", lights.size());
    for (int i = 0; i < lights.size(); i++)
    {
        auto& light = lights[i];
        setUniform("point_light[" + std::to_string(i) + "].pos", light.position);
        setUniform("point_light[" + std::to_string(i) + "].val", light.intensity);
        setUniform("point_light[" + std::to_string(i) + "].width", light.width);
    }
}

void Shader::setMVP(const glm::mat4& view, const glm::mat4& projection)
{
    setUniform("view", view);
    setUniform("projection", projection);
}

void Shader::setCamera(const Camera2& camera)
{
    setMVP(camera.view, camera.projection);
    setUniform("camera_pos", camera.cam_pos);
    setUniform("near", camera.near);
    setUniform("far", camera.far);
}

GLuint Shader::id()
{
    return sp_;
}

void Shader::setTexture(const std::string& name, Texture* texture)
{
    textures[name] = texture;
}

void Shader::solveTextures()
{
    int idx = 0;
    for (auto& [k, v] : textures)
    {
        v->use(idx);
        setUniformi(k, idx);
        idx++;
    }
}

#pragma endregion

#pragma region UniformBuffer

UniformBuffer::UniformBuffer()
{
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
}

void UniformBuffer::use(int id)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, id, ubo);
}

void UniformBuffer::setData(int size, const void* value, GLuint mode)
{
    glBufferData(GL_UNIFORM_BUFFER, size, value, mode);
}

#pragma endregion

#pragma region FramebufferObject

FramebufferObject::FramebufferObject(const std::vector<Texture2D*>& attachments, int width, int height, const Texture2D* depth_attachments) : width_(width), height_(height)
{
    glGenFramebuffers(1, &fb_);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_);

    for (int i = 0; i < attachments.size(); i++)
    {
        atts.push_back(GL_COLOR_ATTACHMENT0 + i);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, attachments[i]->id(), 0);
    }
    glDrawBuffers(attachments.size(), atts.data());

    if (depth_attachments == nullptr)
    {
        glGenRenderbuffers(1, &rbo_);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_);
    }
    else
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachments->id(), 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FramebufferObject::FramebufferObject(const std::vector<TextureCube*>& attachments, int width, int height, int cube_idx, const TextureCube* depth_attachments) : width_(width), height_(height)
{
    glGenFramebuffers(1, &fb_);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_);


    for (int i = 0; i < attachments.size(); i++)
    {
        atts.push_back(GL_COLOR_ATTACHMENT0 + i);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cube_idx, attachments[i]->id(), 0);
    }
    glDrawBuffers(attachments.size(), atts.data());

    if (depth_attachments == nullptr)
    {
        glGenRenderbuffers(1, &rbo_);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_);
    }
    else
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cube_idx, depth_attachments->id(), 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FramebufferObject::use()
{
    glViewport(0, 0, width_, height_);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_);
    glDrawBuffers(atts.size(), atts.data());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

#pragma endregion

#pragma region Material

std::map<std::string, Texture2D*> Material::texture_pool;

void Material::loadTexturesAssimpType(aiTextureType type, const std::string& type_name, aiMaterial* mat, const std::string& dir)
{
    for (int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString filename;
        mat->GetTexture(type, i, &filename);
        std::string full_name = dir + filename.C_Str();
        Texture2D* texture;
        if (texture_pool.find(full_name) == texture_pool.end())
        {
            texture = new Texture2D(full_name);
            texture_pool[full_name] = texture;
        }
        else
        {
            texture = texture_pool[full_name];
        }
        textures[type_name + std::to_string(i + 1)] = texture;
        std::string flag_name = "usetex" + type_name.substr(7);
        properties_i[flag_name] = 1;
    }
}

void Material::loadTexturesAssimp(aiMaterial* mat, const std::string& dir)
{
    float t;
    if (AI_SUCCESS != mat->Get(AI_MATKEY_SHININESS, t))
        t = 1.0f;
    properties_f["shininess"] = t;

    glm::vec3 tv3;
    aiColor3D aitv3;

    if (AI_SUCCESS != mat->Get(AI_MATKEY_COLOR_AMBIENT, aitv3))
        tv3 = glm::vec3(0.0f);
    else
        tv3.x = aitv3.r, tv3.y = aitv3.g, tv3.z = aitv3.b;
    properties_v3["color_ambient"] = tv3;
    properties_i["usetex_ambient"] = 0;

    if (AI_SUCCESS != mat->Get(AI_MATKEY_COLOR_DIFFUSE, aitv3))
        tv3 = glm::vec3(0.5f);
    else
        tv3.x = aitv3.r, tv3.y = aitv3.g, tv3.z = aitv3.b;
    properties_v3["color_diffuse"] = tv3;
    properties_i["usetex_diffuse"] = 0;

    if (AI_SUCCESS != mat->Get(AI_MATKEY_COLOR_SPECULAR, aitv3))
        tv3 = glm::vec3(0.5f);
    else
        tv3.x = aitv3.r, tv3.y = aitv3.g, tv3.z = aitv3.b;
    properties_v3["color_specular"] = tv3;
    properties_i["usetex_specular"] = 0;

    loadTexturesAssimpType(aiTextureType_AMBIENT, "texture_ambient", mat, dir);
    loadTexturesAssimpType(aiTextureType_DIFFUSE, "texture_diffuse", mat, dir);
    loadTexturesAssimpType(aiTextureType_SPECULAR, "texture_specular", mat, dir);
}

Material::Material()
{
}

Material::Material(aiMaterial* mat, const std::string& dir)
{
    // load from material file, directly load textures
    loadTexturesAssimp(mat, dir);
}

void Material::load(aiMaterial* mat, const std::string& dir)
{
    // load from material file, directly load textures
    loadTexturesAssimp(mat, dir);
}

void Material::use(Shader& shader)
{
    // put textures into shader
    GLuint shader_id = shader.id();
    GLuint texture_unit_id = 4; // first 4 units are reserved
    for (GLuint i = texture_unit_id; i < texture_unit_id + 8; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    for (auto& [k, v] : textures)
    {
        shader.setUniformi(k, texture_unit_id);
        v->use(texture_unit_id);
        texture_unit_id++;
    }
    for (auto& [k, v] : properties_f)
    {
        shader.setUniform(k, v);
    }
    for (auto& [k, v] : properties_i)
    {
        shader.setUniformi(k, v);
    }
    for (auto& [k, v] : properties_v3)
    {
        shader.setUniform(k, v);
    }
}

#pragma endregion

#pragma region Mesh

void Mesh2::loadMeshAssimp(aiMesh* mesh, const aiScene* scene, const std::string& dir)
{
    for (int j = 0; j < mesh->mNumVertices; j++)
    {
        vertices_.push_back(mesh->mVertices[j].x);
        vertices_.push_back(mesh->mVertices[j].y);
        vertices_.push_back(mesh->mVertices[j].z);
        vertices_.push_back(mesh->mNormals[j].x);
        vertices_.push_back(mesh->mNormals[j].y);
        vertices_.push_back(mesh->mNormals[j].z);
        if (mesh->mTextureCoords[0])
        {
            vertices_.push_back(mesh->mTextureCoords[0][j].x);
            vertices_.push_back(mesh->mTextureCoords[0][j].y);
        }
        else
        {
            vertices_.push_back(0);
            vertices_.push_back(0);
        }
        vertices_.push_back(mesh->mTangents[j].x);
        vertices_.push_back(mesh->mTangents[j].y);
        vertices_.push_back(mesh->mTangents[j].z);
        vertices_.push_back(mesh->mBitangents[j].x);
        vertices_.push_back(mesh->mBitangents[j].y);
        vertices_.push_back(mesh->mBitangents[j].z);
    }

    for (int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices; j++)
        {
            indices_.push_back(face.mIndices[j]);
        }
    }

    material_.load(scene->mMaterials[mesh->mMaterialIndex], dir);

    n_ = vertices_.size();
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(GLfloat), vertices_.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint), indices_.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void*)(11 * sizeof(GLfloat)));
    glEnableVertexAttribArray(4);
}

Mesh2::Mesh2()
{
}

Mesh2::Mesh2(aiMesh* mesh, const aiScene* scene, const std::string& dir)
{
    loadMeshAssimp(mesh, scene, dir);
}

void Mesh2::draw(Shader& shader)
{
    material_.use(shader);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    shader.solveTextures();
    glDrawElements(GL_TRIANGLES, n_, GL_UNSIGNED_INT, 0);
}

#pragma endregion

#pragma region Model

void Model2::loadModelAssimp(const std::string& filename)
{
    Assimp::Importer importer;
    auto scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    std::string dir = filename.substr(0, filename.find_last_of('/') + 1);
    std::function<void(aiNode*, const aiScene*)> solve = [&](aiNode* node, const aiScene* scene)
        {
            for (int i = 0; i < node->mNumMeshes; i++)
            {
                meshes.push_back(Mesh2(scene->mMeshes[node->mMeshes[i]], scene, dir));
            }
            for (int i = 0; i < node->mNumChildren; i++)
            {
                aiNode* child = node->mChildren[i];
                solve(child, scene);
            }
        };
    solve(scene->mRootNode, scene);
}

Model2::Model2()
{
}

Model2::Model2(const std::string& filename)
{
    loadModelAssimp(filename);
}

void Model2::addChildren(std::shared_ptr<Model2> model, const glm::mat4& model_matrix)
{
    children.push_back({ model, model_matrix });
}

void Model2::draw(Shader& shader, const glm::mat4& model_matrix)
{
    shader.setUniform("model", model_matrix);
    for (auto& i : meshes)
    {
        i.draw(shader);
    }
    for (auto& i : children)
    {
        i.first->draw(shader, i.second * model_matrix);
    }
}

#pragma endregion

#pragma region SceneDesc

SceneDesc::SceneDesc()
{
}

void SceneDesc::load(std::shared_ptr<Object> obj)
{
    std::function<void(std::shared_ptr<Object>, glm::mat4)> solve = [&](std::shared_ptr<Object> p, glm::mat4 model)
        {
            if (std::dynamic_pointer_cast<Model2>(p))
            {
                models.addChildren(std::dynamic_pointer_cast<Model2>(p), model);
            }
            if (std::dynamic_pointer_cast<PointLight>(p))
            {
                PointLight tmp = *std::dynamic_pointer_cast<PointLight>(p);
                tmp.position = glm::vec3(model * glm::vec4(tmp.position, 1.0f));
                point_lights.push_back(tmp);
            }
            for (auto [q, o] : p->children)
            {
                solve(q, model * o);
            }
        };

    solve(obj, glm::mat4(1.0));
}

SceneDesc::SceneDesc(std::shared_ptr<Object> obj)
{
    load(obj);
}

SceneDesc::SceneDesc(Scene& scene)
{
    load(scene.root);
    ambient_light_irradiance = scene.ambient_light_irradiance;
}

#pragma endregion

#pragma region ShadowMapper

ShadowMapper::ShadowMapper() : shadow_shader("ExampleData/shaders/TestComplex/shadow.vs", "ExampleData/shaders/TestComplex/shadow.fs"),
depth_texture(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT),
pos_texture(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, GL_RGB16F, GL_RGB, GL_FLOAT),
normal_texture(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, GL_RGB16F, GL_RGB, GL_FLOAT),
flux_texture(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, GL_RGB16F, GL_RGB, GL_FLOAT),
shadow_map_fbo{
    {{&pos_texture, &normal_texture, &flux_texture}, (int)SHADOW_MAP_WIDTH, (int)SHADOW_MAP_HEIGHT, 0, &depth_texture},
    {{&pos_texture, &normal_texture, &flux_texture}, (int)SHADOW_MAP_WIDTH, (int)SHADOW_MAP_HEIGHT, 1, &depth_texture},
    {{&pos_texture, &normal_texture, &flux_texture}, (int)SHADOW_MAP_WIDTH, (int)SHADOW_MAP_HEIGHT, 2, &depth_texture},
    {{&pos_texture, &normal_texture, &flux_texture}, (int)SHADOW_MAP_WIDTH, (int)SHADOW_MAP_HEIGHT, 3, &depth_texture},
    {{&pos_texture, &normal_texture, &flux_texture}, (int)SHADOW_MAP_WIDTH, (int)SHADOW_MAP_HEIGHT, 4, &depth_texture},
    {{&pos_texture, &normal_texture, &flux_texture}, (int)SHADOW_MAP_WIDTH, (int)SHADOW_MAP_HEIGHT, 5, &depth_texture} }
{
    depth_texture.setParami(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    depth_texture.setParami(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   // depth_texture.setParami(GL_GENERATE_MIPMAP, GL_FALSE);
    pos_texture.setParami(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    pos_texture.setParami(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    normal_texture.setParami(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    normal_texture.setParami(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    flux_texture.setParami(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    flux_texture.setParami(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // glGenFramebuffers(1, &shadow_map_fbo);
    // glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fbo);

    // auto makeCubemap = [&](GLuint &id, GLuint intformat, GLuint component)
    // {
    //     glGenTextures(1, &id);
    //     glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    //     for (int i = 0; i < 6; i++)
    //         glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, intformat, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 0, component, GL_FLOAT, NULL);
    //     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //     // GLfloat texture_border_color[] = {1.0, 1.0, 1.0, 1.0};
    //     // glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, texture_border_color);
    //     glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    // };

    // makeCubemap(depth_texture, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
    // makeCubemap(pos_texture, GL_RGB16F, GL_RGB);
    // makeCubemap(normal_texture, GL_RGB16F, GL_RGB);
    // makeCubemap(flux_texture, GL_RGB16F, GL_RGB);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapper::lightPass(glm::vec3 shadow_light_pos, glm::vec3 shadow_light_int, Model2* scene)
{
    shadow_shader.use();
    glm::mat4 shadow_light_view[6], shadow_light_projection;
    shadow_light_view[0] = glm::lookAt(shadow_light_pos, shadow_light_pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0, -1.0, 0.0));
    shadow_light_view[1] = glm::lookAt(shadow_light_pos, shadow_light_pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0, -1.0, 0.0));
    shadow_light_view[2] = glm::lookAt(shadow_light_pos, shadow_light_pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0, 0.0, 1.0));
    shadow_light_view[3] = glm::lookAt(shadow_light_pos, shadow_light_pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0, 0.0, -1.0));
    shadow_light_view[4] = glm::lookAt(shadow_light_pos, shadow_light_pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0, -1.0, 0.0));
    shadow_light_view[5] = glm::lookAt(shadow_light_pos, shadow_light_pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0, -1.0, 0.0));
    shadow_light_projection = glm::perspective(glm::radians(90.0), 1.0, 0.01, 100.0);

    shadow_shader.setUniform("model", (glm::mat4(1.0f)));
    shadow_shader.setUniform("light_pos", shadow_light_pos);
    shadow_shader.setUniform("light_int", shadow_light_int);
    shadow_shader.setUniform("shadow_limit", shadow_limit);

    for (int i = 0; i < 6; i++)
    {
        shadow_shader.setUniform("shadow_light_view_project", shadow_light_projection * shadow_light_view[i]);
        shadow_map_fbo[i].use();
        // glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
        // glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fbo.fbo_);
        // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, depth_texture.id(), 0);
        // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, pos_texture.id(), 0);
        // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, normal_texture.id, 0);
        // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, flux_texture, 0);
        // GLuint tmp[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        // glDrawBuffers(3, tmp);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene->draw(shadow_shader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapper::attach(Shader& shader)
{
    shader.setUniform("shadow_limit", shadow_limit);
    // glActiveTexture(GL_TEXTURE31);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, depth_texture);
    // shader.setUniformi("shadow_map", 31);
    // glActiveTexture(GL_TEXTURE30);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, pos_texture);
    // shader.setUniformi("shadow_map_pos", 30);
    // glActiveTexture(GL_TEXTURE29);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, normal_texture);
    // shader.setUniformi("shadow_map_normal", 29);
    // glActiveTexture(GL_TEXTURE28);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, flux_texture);
    // shader.setUniformi("shadow_map_flux", 28);
    shader.setTexture("shadow_map", &depth_texture);
    shader.setTexture("shadow_map_pos", &pos_texture);
    shader.setTexture("shadow_map_normal", &normal_texture);
    shader.setTexture("shadow_map_flux", &flux_texture);
}

#pragma endregion

#pragma region Deffered

Deferred::Deferred(int width, int height) : width_(width), height_(height), gbuffer_shader("ExampleData/shaders/TestComplex//gbuf.vs", "ExampleData/shaders/TestComplex//gbuf.fs"),
gbuffer_texture{
    {width, height},
    {width, height},
    {width, height},
    {width, height},
    {width, height},
    {width, height} },
    gbuffer_fbo(std::vector<Texture2D*>({ gbuffer_texture + 0,
                                          gbuffer_texture + 1,
                                          gbuffer_texture + 2,
                                          gbuffer_texture + 3,
                                          gbuffer_texture + 4,
                                          gbuffer_texture + 5 }),
                                          width, height)
{
    vertices = { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
}

void Deferred::drawGeometry(Model2* scene, const Camera2& camera)
{
    gbuffer_fbo.use();
    gbuffer_shader.use();
    gbuffer_shader.setCamera(camera);
    scene->draw(gbuffer_shader);
}

void Deferred::drawLighting(Shader& lighting_shader)
{
    // set your fbo before calling this function
    lighting_shader.use();
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    for (int i = 0; i < n_gbuffer; i++)
    {
        lighting_shader.setTexture("gbuf" + std::to_string(i), gbuffer_texture + i);
    }
    lighting_shader.solveTextures();
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}
#pragma endregion
