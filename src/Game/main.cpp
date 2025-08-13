#include "stdafx.h"

#include "NewTest001.h"
#include "NewTest002.h"
#include "NewTest003.h"
#include "NewTest004.h"
#include "NewTest005.h"

#include "GameApp3.h"
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
	//создать прогу куда перенести старый код и старые тесты 
	//создать для туториалов

	//NewTest001 game;
	//NewTest002 game;
	//NewTest003 game;
	//NewTest004 game;
	//NewTest005 game;

	//TempApp game;

	//GameApp game;
	//GameApp2 game;
	GameApp3 game;
	game.Run();
}
//=============================================================================