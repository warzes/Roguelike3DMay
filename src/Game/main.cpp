#include "stdafx.h"
#include "GameApp.h"
#include "TestBlinnPhong.h"
#include "TestModelLoading.h"
#include "TestShadowMapping.h"
//=============================================================================
#if defined(_MSC_VER)
#	pragma comment( lib, "3rdparty.lib" )
#	pragma comment( lib, "Engine.lib" )
#endif
//=============================================================================
int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	//GameApp game;

	//TestBlinnPhong game;
	//TestModelLoading game;
	TestShadowMapping game;
	game.Run();
}
//=============================================================================