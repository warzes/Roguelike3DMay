#pragma once

namespace gl4f
{
	class ScopedDebugMarker final
	{
	public:
		ScopedDebugMarker(const char* message);
		ScopedDebugMarker(const ScopedDebugMarker&) = delete;
		~ScopedDebugMarker();
	};
}