#include "stdafx.h"
#include "GameAppOld.h"
#include "TestBlinnPhong.h"
#include "TestModelLoading.h"
#include "TestShadowMapping.h"
#include "TestSkyboxWithCube.h"
#include "TestDeferredSSAO.h"
#include "TestPBR.h"
#include "TestBloom.h"
// test 2
#include "TestSimple.h"
#include "TestForwardPlus.h"

#include "TestComplex.h"

#include "GameApp.h"
#include "GameApp2.h"
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
	//GameAppOld game;

	TestBlinnPhong game;
	//TestModelLoading game;
	//TestShadowMapping game;
	//TestSkyboxWithCube game;
	//TestDeferredSSAO game;
	//TestPBR game;
	//TestBloom game;

	//TestSimple game;
	//TestForwardPlus game;

	//TestComplex game;

	//GameApp game;
	//GameApp2 game;
	//GameApp3 game;
	game.Run();
}
//=============================================================================