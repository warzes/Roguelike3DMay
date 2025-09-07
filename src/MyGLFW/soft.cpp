#include "glad/gl.h"
#include "myglfw3.h"
#include <iostream>
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

#if defined(_MSC_VER)
#	pragma comment( lib, "3rdparty.lib" )
#endif

const char* computeShaderSource = R"(
#version 430

// Размер группы: 16x16 потоков
layout(local_size_x = 16, local_size_y = 16) in;

// Выходные изображения
layout(binding = 0, rgba8) uniform image2D img_color;  // Цвет
layout(binding = 1, r32f) uniform image2D img_depth;   // Глубина (Z)

// Матрицы и разрешение
uniform mat4 u_viewProj;
uniform mat4 u_model;
uniform ivec2 u_resolution;

// === Вершины куба (в локальных координатах) ===
vec3 cubeVertices[8] = vec3[](
    vec3(-0.5, -0.5, -0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3( 0.5,  0.5, -0.5),
    vec3(-0.5,  0.5, -0.5),
    vec3(-0.5, -0.5,  0.5),
    vec3( 0.5, -0.5,  0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3(-0.5,  0.5,  0.5)
);

// === Индексы треугольников (12 треугольников = 6 граней × 2) ===
int indices[36] = int[](
    // -Z
    0, 1, 2,
    0, 2, 3,
    // +Z
    5, 4, 7,
    5, 7, 6,
    // -X
    4, 0, 3,
    4, 3, 7,
    // +X
    1, 5, 6,
    1, 6, 2,
    // -Y
    4, 5, 1,
    4, 1, 0,
    // +Y
    3, 2, 6,
    3, 6, 7
);

// === Цвета граней ===
vec3 getFaceColor(int faceIndex) {
    if (faceIndex == 0) return vec3(1.0, 0.0, 0.0); // -Z: красный
    if (faceIndex == 1) return vec3(0.0, 1.0, 0.0); // +Z: зелёный
    if (faceIndex == 2) return vec3(0.0, 0.0, 1.0); // -X: синий
    if (faceIndex == 3) return vec3(1.0, 1.0, 0.0); // +X: жёлтый
    if (faceIndex == 4) return vec3(1.0, 0.0, 1.0); // -Y: пурпурный
    if (faceIndex == 5) return vec3(0.0, 1.0, 1.0); // +Y: голубой
    return vec3(1.0); // белый (ошибка)
}

// === Барицентрические координаты ===
vec3 barycentric(vec2 p, vec2 a, vec2 b, vec2 c) {
    vec2 v0 = b - a, v1 = c - a, v2 = p - a;
    float den = v0.x * v1.y - v1.x * v0.y;
    if (abs(den) < 1e-6) return vec3(-1.0);
    float invDen = 1.0 / den;
    float w = (v0.x * v2.y - v2.x * v0.y) * invDen;
    float v = (v2.x * v1.y - v1.x * v2.y) * invDen;
    float u = 1.0 - v - w;
    return vec3(u, v, w);
}

void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

    // Проверка границ
    if (pixel.x >= u_resolution.x || pixel.y >= u_resolution.y) {
        return;
    }

    // === Шаг 1: Очистка Z-буфера (каждый поток очищает свой пиксель) ===
    imageStore(img_depth, pixel, vec4(1.0));

    // Делаем барьер, чтобы Z-буфер был полностью очищен
    // ⚠️ barrier() работает ТОЛЬКО внутри work group!
    // Поэтому лучше очищать в отдельном dispatch.
    // Сейчас — упрощённо: очистка и рендер в одном проходе (не идеально, но работает)
    // Рекомендуется: отдельный dispatch для очистки — см. ниже
    memoryBarrierImage();
    barrier(); // ждём всех в группе

    // === Шаг 2: Проекция вершин куба ===
    vec4 clipPos[8];
    for (int i = 0; i < 8; ++i) {
        vec4 world = u_model * vec4(cubeVertices[i], 1.0);
        clipPos[i] = u_viewProj * world;
    }

    vec2 screenPos[8];
    for (int i = 0; i < 8; ++i) {
        if (clipPos[i].w <= 0.0) continue;
        vec2 ndc = clipPos[i].xy / clipPos[i].w;
        screenPos[i] = (ndc * 0.5 + 0.5) * vec2(u_resolution);
    }

    // === Шаг 3: Проверка каждого треугольника ===
    for (int tri = 0; tri < 36; tri += 3) {
        int i0 = indices[tri + 0];
        int i1 = indices[tri + 1];
        int i2 = indices[tri + 2];

        vec2 v0 = screenPos[i0];
        vec2 v1 = screenPos[i1];
        vec2 v2 = screenPos[i2];

        vec2 p = vec2(pixel);
        vec3 b = barycentric(p, v0, v1, v2);

        // Пропускаем, если вне треугольника
        if (b.x < 0.0 || b.y < 0.0 || b.z < 0.0) continue;

        // === Интерполяция Z (в clip space) ===
        float z0 = clipPos[i0].z / clipPos[i0].w;
        float z1 = clipPos[i1].z / clipPos[i1].w;
        float z2 = clipPos[i2].z / clipPos[i2].w;
        float interpolatedZ = b.x * z0 + b.y * z1 + b.z * z2;
        float depth = (interpolatedZ + 1.0) * 0.5; // [0,1]

        // === Z-тест ===
        float existingZ = imageLoad(img_depth, pixel).r;
        if (depth >= existingZ) continue;

        // === Определяем грань и цвет ===
        int faceIndex = tri / 6;
        vec3 color = getFaceColor(faceIndex);

        // === Записываем цвет и глубину ===
        imageStore(img_color, pixel, vec4(color, 1.0));
        imageStore(img_depth, pixel, vec4(depth));
        // НЕТ return — пусть другие треугольники тоже проверяются!
    }

    // === Фон (нижняя полоса для отладки) ===
    if (pixel.y < 20) {
        imageStore(img_color, pixel, vec4(1.0, 0.0, 1.0, 1.0)); // пурпурная полоса
    }
}
)";

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoords = aTexCoords;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
void main() {
    FragColor = texture(screenTexture, TexCoords);
}
)";

// Компиляция шейдера
GLuint compileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compile error: " << infoLog << std::endl;
    }
    return shader;
}

int main() {
    // === Инициализация GLFW ===
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Compute Shader Cube (Debug)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // === Создание текстуры рендера ===
    GLuint renderTexture;
    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindImageTexture(0, renderTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    // === Создание Z-буфера ===
    GLuint depthTexture;
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 800, 600, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTexture(1, depthTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

    // === Compile compute shader ===
    GLuint computeShader = compileShader(computeShaderSource, GL_COMPUTE_SHADER);
    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);

    int linked;
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(computeProgram, 512, nullptr, log);
        std::cerr << "Compute program link error: " << log << std::endl;
        return -1;
    }

    // === Экранная отрисовка (full-screen quad) ===
    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    GLuint screenProgram = glCreateProgram();
    glAttachShader(screenProgram, vertexShader);
    glAttachShader(screenProgram, fragmentShader);
    glLinkProgram(screenProgram);

    float quadVertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,

        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };

    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // === Настройка матриц ===
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 10.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 viewProj = proj * view;

    float time = 0.0f;

    // === Основной цикл ===
    while (!glfwWindowShouldClose(window)) {
        time += 0.001f;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // === Модельная матрица (вращение) ===

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), time, glm::vec3(1.0f, 1.0f, 0.0f));
       // glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 1.0f, 0.0f));


        // === Запуск compute shader ===
        glUseProgram(computeProgram);
        glUniformMatrix4fv(glGetUniformLocation(computeProgram, "u_model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(computeProgram, "u_viewProj"), 1, GL_FALSE, glm::value_ptr(viewProj));
        glUniform2i(glGetUniformLocation(computeProgram, "u_resolution"), 800, 600);


        glDispatchCompute(800 / 16, 600 / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // === Отображение результата ===
        glUseProgram(screenProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        glUniform1i(glGetUniformLocation(screenProgram, "screenTexture"), 0);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // === Очистка ===
    glDeleteProgram(computeProgram);
    glDeleteProgram(screenProgram);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteTextures(1, &renderTexture);
    glDeleteTextures(1, &depthTexture);
    glfwTerminate();
    return 0;
}