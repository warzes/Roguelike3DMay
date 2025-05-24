#pragma once

namespace gl4
{
	class ScopedDebugMarker final
	{
	public:
		ScopedDebugMarker(const char* message);
		ScopedDebugMarker(const ScopedDebugMarker&) = delete;
		ScopedDebugMarker(ScopedDebugMarker&&) = default;
		~ScopedDebugMarker();

		ScopedDebugMarker& operator=(const ScopedDebugMarker&) = delete;
	};
} // namespace gl4