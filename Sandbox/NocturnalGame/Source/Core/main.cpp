
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#include "Core/EngineCore.h"
#include "Core/Game.h"

//NocturnalEngine::Application* NocturnalEngine::CreateApplication()
//{
//	return new NocturnalEngine::Application();
//}

#ifdef NOCTURNAL_PLATFORM_WINDOWS

//extern NocturnalEngine::Application* NocturnalEngine::CreateApplication();

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

#if DEBUG
    // Memory leak check (Debug only)
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);

    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_SHOW);
#else
    // Hide console in Release
    //HWND hWnd = GetConsoleWindow();
    //if (hWnd)
    //    ShowWindow(hWnd, SW_HIDE);
#endif
	// temporary memory leak check to set break point on specific allocation
	//_CrtSetBreakAlloc();


	NocturnalEngine::Log::Init();
	auto app = NocturnalEngine::CreateEngineCore();
    NocturnalEngine::NocturnalGameLayer game;
    app->SetGameLayer(&game);
	app->Run();
	delete app;

}
#endif