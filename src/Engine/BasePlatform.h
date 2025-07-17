#pragma once

#if defined(_MSC_VER)
#endif
#if defined(__GNUC__) || defined(__clang__)
#endif

#define PLATFORM_WINDOWS 0
#define PLATFORM_ANDROID 0
#define PLATFORM_EMSCRIPTEN 0

#if defined(_WIN32)
#	undef PLATFORM_WINDOWS
#	define PLATFORM_WINDOWS 1
#endif
#if defined(__ANDROID__)
#	undef PLATFORM_ANDROID
#	define PLATFORM_ANDROID 1
#endif
#if defined(__EMSCRIPTEN__)
#	undef PLATFORM_EMSCRIPTEN
#	define PLATFORM_EMSCRIPTEN 1
#endif