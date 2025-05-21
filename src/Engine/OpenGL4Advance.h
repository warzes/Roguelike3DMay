#pragma once

#include "OpenGL4Simple.h"

namespace gl4A
{
	class ScopedDebugMarker final
	{
	public:
		ScopedDebugMarker(const char* message);
		ScopedDebugMarker(const ScopedDebugMarker&) = delete;
		~ScopedDebugMarker();
	};
} // namespace gl4A