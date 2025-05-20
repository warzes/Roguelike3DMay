#include "stdafx.h"
#include "OpenGL4DebugMarker.h"
//=============================================================================
gl4::ScopedDebugMarker::ScopedDebugMarker(const char* message)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, message);
}
//=============================================================================
gl4::ScopedDebugMarker::~ScopedDebugMarker()
{
	glPopDebugGroup();
}
//=============================================================================