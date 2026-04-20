#include <3ds.h>
#include <stdlib.h>
#include <stdio.h>
#include <i18n.hpp>
#include <sys/stat.h>

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

#include <vector>
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    i32 renderResult = 0;

    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

	printf("\x1b[1;1HEoSD 3DS unofficial port. Based on:");
	printf("\x1b[2;1Hgithub.com/GensokyoClub/th06 (portable)");
	printf("\x1b[3;1HTouhou Project - (c) Team Shanghai Alice");
	printf("\x1b[5;1HPre-launch filesystem check...");
    gfxSwapBuffers();
    gspWaitForVBlank();
    aptInit();

    chdir("/");
    mkdir("3ds", 0777);
    chdir("3ds");
    mkdir("eosd3ds", 0777);
	chdir("eosd3ds");
    
    std::vector<const char*> files = {
        TH_CM_DAT_FILE,
        TH_ED_DAT_FILE,
        TH_IN_DAT_FILE,
        TH_MD_DAT_FILE,
        TH_ST_DAT_FILE,
        TH_TL_DAT_FILE
    };
    for (const char* file : files) {
        printf("\x1b[6;1Hstat sdmc:/3ds/eosd3ds/%s ...", file);
        gfxSwapBuffers();
        gspWaitForVBlank();
        if (access(file, F_OK) < 0) {
            printf("\x1b[6;1HError: sdmc:/3ds/eosd3ds/%s not found. Refer to the README for more information.\n", file);
            printf("Press START to exit.");
            gfxSwapBuffers();
            while (aptMainLoop()) {
                hidScanInput();
                u32 kDown = hidKeysDown();
                if (kDown & KEY_START) break;
                gspWaitForVBlank();
            }
            return 1;
        }
    }
    for(int i = 1; i < 17; i++){
        char file[15];
        sprintf(file, "bgm/th06_%02d.wav", i);
        printf("\x1b[6;1Hstat sdmc:/3ds/eosd3ds/bgm/th06_%02d.wav ...", i);
        gfxSwapBuffers();
        gspWaitForVBlank();
        if (access(file, F_OK) < 0) {
            printf("\x1b[6;1HError: sdmc:/3ds/eosd3ds/bgm/th06_%02d.wav not found. Refer to the README for more information.\n", i);
            printf("Press START to exit.");
            gfxSwapBuffers();
            while (aptMainLoop()) {
                hidScanInput();
                u32 kDown = hidKeysDown();
                if (kDown & KEY_START) break;
                gspWaitForVBlank();
            }
            return 1;
        }
    }

    printf("\x1b[8;1H;Started execution.\n");

    gspWaitForVBlank();

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

    g_GameWindow.curFrame = 0;

    while (aptMainLoop())
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
        gspWaitForVBlank();
    }

stop:
    printf("renderResult is different than 0. (may exit or restart rendering)\n");
    g_Chain.Release();
    g_SoundPlayer.Release();

    delete g_AnmManager;
    g_AnmManager = NULL;

    SDL_DestroyWindow(g_GameWindow.window);

    pglExit();

    SDL_Quit();

    if (renderResult == 2)
    {
        printf("renderResult == 2. Restarting rendering...\n");
        g_GameErrorContext.ResetContext();

        GameErrorContext::Log(&g_GameErrorContext, TH_ERR_OPTION_CHANGED_RESTART);

        goto restart;
    }

    printf("renderResult == 1. Saving before exiting...\n");

    FileSystem::WriteDataToFile(TH_CONFIG_FILE, &g_Supervisor.cfg, sizeof(g_Supervisor.cfg));
    //    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, g_GameWindow.screenSaveActive, NULL, SPIF_SENDCHANGE);
    //    SystemParametersInfo(SPI_SETLOWPOWERACTIVE, g_GameWindow.lowPowerActive, NULL, SPIF_SENDCHANGE);
    //    SystemParametersInfo(SPI_SETPOWEROFFACTIVE, g_GameWindow.powerOffActive, NULL, SPIF_SENDCHANGE);

    g_GameErrorContext.Flush();
    return 1;
}
