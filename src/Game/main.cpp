#include "stdafx.h"
//=============================================================================
#if defined(_MSC_VER)
#	pragma comment( lib, "3rdparty.lib" )
#	pragma comment( lib, "Engine.lib" )
#endif
//=============================================================================
#if defined(_WIN32)
extern "C" {
	_declspec(dllexport) uint32_t NvOptimusEnablement = 1;
	_declspec(dllexport) uint32_t AmdPowerXpressRequestHighPerformance = 1;
}
#endif
//=============================================================================
int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
}
//=============================================================================