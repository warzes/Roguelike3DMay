#include "stdafx.h"
#include "GameApp.h"
#include "TestBlinnPhong.h"
#include "TestModelLoading.h"
#include "TestShadowMapping.h"
#include "TestSkyboxWithCube.h"
#include "TestDeferredSSAO.h"
#include "TestPBR.h"
#include "TestBloom.h"
// test 2
#include "TestSimple.h"
#include "TestTerrain.h"
#include "TestForwardPlus.h"
#include "TestCascadedShadowMaps.h"

#include "TestComplex.h"
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
	//TestShadowMapping game;
	//TestSkyboxWithCube game;
	//TestDeferredSSAO game;
	//TestPBR game;
	//TestBloom game;

	//TestSimple game;
	//TestTerrain game;
	//TestForwardPlus game;
	//TestCascadedShadowMaps game;

	TestComplex game;

	game.Run();
}
//=============================================================================