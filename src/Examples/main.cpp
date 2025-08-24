#include "stdafx.h"
#include "Example001.h"
#include "Example002.h"
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
	// Вывод треугольника на основную поверхность
	// - вершинный буфер из позиции и цвета
	// - индексный буфер
	// - создание GraphicsPipeline
	//Example001 game;


	Example002 game;
	game.Run();
}
//=============================================================================