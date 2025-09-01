#include "stdafx.h"
#include "Demo001.h"
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
	/*
	https://www.youtube.com/shorts/0hdta4b8hcA - идея
	наоборот качественный рендер
	*/
	Demo001 game;
	game.Run();
}
//=============================================================================