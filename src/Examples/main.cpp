#include "stdafx.h"
#include "Example001.h"
#include "Example002.h"
#include "Example003.h"
#include "Example004.h"
#include "Example005.h"
#include "Example006.h"
#include "Example007.h"
#include "Example008.h"
#include "Example009.h"

#include "TM_000.h"
#include "TM_001.h"
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
	//Example001 game;
	//Example002 game;
	//Example003 game;
	//Example004 game;
	//Example005 game;
	//Example006 game;
	//Example007 game;
	//Example008 game;
	//Example009 game;

	//TM_000 game;
	TM_001 game;

	game.Run();
}
//=============================================================================