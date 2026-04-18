#ifdef __3DS__
#include <3ds.h>
#include <stdlib.h>
#include <stdio.h>
#include <i18n.hpp>
#include <sys/stat.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <cstdio>

#include "AnmManager.hpp"
#include "Chain.hpp"
#include "FileSystem.hpp"
#include "GameErrorContext.hpp"
#include "GameWindow.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "Supervisor.hpp"
#include "ZunResult.hpp"
#include "i18n.hpp"
#include "utils.hpp"
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    i32 renderResult = 0;

    #ifdef __3DS__

    gfxInitDefault();
    aptInit();

    chdir("/");
    mkdir("3ds", 0777);
    chdir("3ds");
    mkdir("eosd3ds", 0777);
	chdir("eosd3ds");

    #endif

    //    MSG msg;
    //    i32 waste1, waste2, waste3, waste4, waste5, waste6;

    //    if (utils::CheckForRunningGameInstance())
    //    {
    //        g_GameErrorContext.Flush();
    //
    //        return 1;
    //    }

    //    g_Supervisor.hInstance = hInstance;

    if (g_Supervisor.LoadConfig(TH_CONFIG_FILE) != ZUN_SUCCESS)
    {
        g_GameErrorContext.Flush();
        return -1;
    }

    //    if (GameWindow::InitD3dInterface())
    //    {
    //        g_GameErrorContext.Flush();
    //        return 1;
    //    }

    //    SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &g_GameWindow.screenSaveActive, 0);
    //    SystemParametersInfo(SPI_GETLOWPOWERACTIVE, 0, &g_GameWindow.lowPowerActive, 0);
    //    SystemParametersInfo(SPI_GETPOWEROFFACTIVE, 0, &g_GameWindow.powerOffActive, 0);
    //    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0, NULL, SPIF_SENDCHANGE);
    //    SystemParametersInfo(SPI_SETLOWPOWERACTIVE, 0, NULL, SPIF_SENDCHANGE);
    //    SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0, NULL, SPIF_SENDCHANGE);

restart:
    GameWindow::CreateGameWindow();
    consoleInit(GFX_BOTTOM, NULL);


	printf("\x1b[3;1H----------------------------------------");
	printf("\x1b[4;1HEoSD 3DS unofficial port. Based on:");
	printf("\x1b[5;1Hhttps://github.com/GensokyoClub/th06/tree/portable");
	printf("\x1b[6;1HTouhou Project - (c) Team Shanghai Alice");

    g_AnmManager = new AnmManager();

    if (GameWindow::InitD3dRendering())
    {
        g_GameErrorContext.Flush();
        return 1;
    }

    g_SoundPlayer.InitializeDSound();
    Controller::GetJoystickCaps();
    Controller::ResetKeyboard();

    if (Supervisor::RegisterChain() != ZUN_SUCCESS)
    {
        goto stop;
    }

    #ifndef __3DS__
    if (!g_Supervisor.cfg.windowed)
    {
        SDL_ShowCursor(SDL_DISABLE);
    }
    #endif

    g_GameWindow.curFrame = 0;

    while (
        #ifdef __3DS__
        aptMainLoop()
        #else
        true
        #endif
    )
    {
        SDL_Event e;

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                goto stop;
            }
        }
        renderResult = g_GameWindow.Render();
        if (renderResult != 0)
        {
            break;
        }
        #ifdef __3DS__
        gspWaitForVBlank();
        #endif
        //        SDL_Delay(1000.0f / 60.0f);

        //        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        //        {
        //            TranslateMessage(&msg);
        //            DispatchMessage(&msg);
        //        }
        //        else
        //        {
        //            testCoopLevelRes = g_Supervisor.d3dDevice->TestCooperativeLevel();
        //            if (testCoopLevelRes == D3D_OK)
        //            {
        //                renderResult = g_GameWindow.Render();
        //                if (renderResult != 0)
        //                {
        //                    goto stop;
        //                }
        //            }
        //            else if (testCoopLevelRes == D3DERR_DEVICENOTRESET)
        //            {
        //                g_AnmManager->ReleaseSurfaces();
        //                testResetRes = g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);
        //                if (testResetRes != 0)
        //                {
        //                    goto stop;
        //                }
        //                GameWindow::InitD3dDevice();
        //                g_Supervisor.unk198 = 3;
        //            }
        //        }
    }

stop:
    printf("renderResult is different than 0. (may exit or restart rendering)\n");
    g_Chain.Release();
    g_SoundPlayer.Release();

    delete g_AnmManager;
    g_AnmManager = NULL;

    SDL_DestroyWindow(g_GameWindow.window);

    #ifndef __3DS__
    SDL_GL_DeleteContext(g_GameWindow.glContext);
    #else
    pglExit();
    #endif

    SDL_Quit();

    if (renderResult == 2)
    {
        printf("renderResult == 2. Restarting rendering...\n");
        g_GameErrorContext.ResetContext();

        GameErrorContext::Log(&g_GameErrorContext, TH_ERR_OPTION_CHANGED_RESTART);

        #ifndef __3DS__
        if (!g_Supervisor.cfg.windowed)
        {
            SDL_ShowCursor(SDL_ENABLE);
        }
        #endif

        goto restart;
    }

    printf("renderResult == 1. Saving before exiting...\n");

    FileSystem::WriteDataToFile(TH_CONFIG_FILE, &g_Supervisor.cfg, sizeof(g_Supervisor.cfg));
    //    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, g_GameWindow.screenSaveActive, NULL, SPIF_SENDCHANGE);
    //    SystemParametersInfo(SPI_SETLOWPOWERACTIVE, g_GameWindow.lowPowerActive, NULL, SPIF_SENDCHANGE);
    //    SystemParametersInfo(SPI_SETPOWEROFFACTIVE, g_GameWindow.powerOffActive, NULL, SPIF_SENDCHANGE);

    #ifndef __3DS__
    SDL_ShowCursor(SDL_ENABLE);
    #endif
    g_GameErrorContext.Flush();
    return 0;
}
