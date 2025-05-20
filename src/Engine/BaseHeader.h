﻿#pragma once

#include "3rdparty/3rdpartyConfig.h"
#include "EngineConfig.h"
#include "BasePlatform.h"

#define _USE_MATH_DEFINES

#if defined(_WIN32)
#	define NOMINMAX
#	define WIN32_LEAN_AND_MEAN
#endif

#if defined(_MSC_VER)
#	pragma warning(push, 3)
#	pragma warning(disable : 4061)
#	pragma warning(disable : 4365)
#	pragma warning(disable : 4464)
#	pragma warning(disable : 4514)
#	pragma warning(disable : 4619)
#	pragma warning(disable : 4820)
#	pragma warning(disable : 5029)
#	pragma warning(disable : 5267)
#endif

#include <cmath>
#include <cctype>
#include <cwctype>
#include <algorithm>
//#include <chrono>
#include <random>
#include <filesystem>
#include <string>
#include <string_view>
#include <span>
#include <array>
#include <stack>

#include <glad/gl.h>
#include <glfw/glfw3.h>

#if defined(__EMSCRIPTEN__)
#	define GLFW_INCLUDE_ES3
#	include <emscripten/emscripten.h>
#	include <emscripten/html5.h>
#endif

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#define GLM_FORCE_XYZW_ONLY 1
#define GLM_FORCE_LEFT_HANDED 1
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/type_aligned.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <stb/stb_image_resize2.h>
#include <stb/stb_truetype.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>
#include <assimp/GltfMaterial.h>

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif