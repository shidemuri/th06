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

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <vector>
#include <fat.h>

static void *__xfb = NULL;
static GXRModeObj *__rmode = NULL;
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    i32 renderResult = 0;
    VIDEO_Init();

	// This function initialises the attached controllers
	WPAD_Init();

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	__rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	__xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(__rmode));

	// Initialise the console, required for printf
	console_init(__xfb,20,20,__rmode->fbWidth,__rmode->xfbHeight,__rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(__rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(__xfb);

	// Make the display visible
	VIDEO_SetBlack(false);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();

	printf("\x1b[1;1HEoSD Wii unofficial port. Based on:");
	printf("\x1b[2;1Hgithub.com/GensokyoClub/th06 (portable)");
	printf("\x1b[3;1HTouhou Project - (c) Team Shanghai Alice");
	printf("\x1b[5;1HPre-launch filesystem check...");
	VIDEO_WaitVSync();
    if(!fatInitDefault())
    {
        printf("\x1b[6;1HError: Failed to initialize the SD card. Refer to the README for more information.\n");
        printf("Press HOME to exit.");
        VIDEO_WaitVSync();
        while (SYS_MainLoop()) {
            WPAD_ScanPads();
            u32 kDown = WPAD_ButtonsDown(0);
            if (kDown & WPAD_BUTTON_HOME) break;
            VIDEO_WaitVSync();
        }
        return 1;
    }

    mkdir("sd:/th06", 0777);
    chdir("sd:/th06");
    
    std::vector<const char*> files = {
        TH_CM_DAT_FILE,
        TH_ED_DAT_FILE,
        TH_IN_DAT_FILE,
        TH_MD_DAT_FILE,
        TH_ST_DAT_FILE,
        TH_TL_DAT_FILE
    };
    for (const char* file : files) {
        printf("\x1b[6;1Hstat sd:/th06/%s ...", file);
        VIDEO_WaitVSync();
        if (access(file, F_OK) < 0) {
            printf("\x1b[6;1HError: sd:/th06/%s not found. Refer to the README for more information.\n", file);
            printf("Press HOME to exit.");
            while (SYS_MainLoop()) {
                WPAD_ScanPads();
                u32 kDown = WPAD_ButtonsDown(0);
                if (kDown & WPAD_BUTTON_HOME) break;
                VIDEO_WaitVSync();
            }
            return 1;
        }
    }
    for(int i = 1; i < 17; i++){
        char file[15];
        sprintf(file, "bgm/th06_%02d.wav", i);
        printf("\x1b[6;1Hstat sd:/th06/bgm/th06_%02d.wav ...", i);
        VIDEO_WaitVSync();
        if (access(file, F_OK) < 0) {
            printf("\x1b[6;1HError: sd:/th06/bgm/th06_%02d.wav not found. Refer to the README for more information.\n", i);
            printf("Press HOME to exit.");
            while (SYS_MainLoop()) {
                WPAD_ScanPads();
                u32 kDown = WPAD_ButtonsDown(0);
                if (kDown & WPAD_BUTTON_HOME) break;
                VIDEO_WaitVSync();
            }
            return 1;
        }
    }

    printf("\x1b[8;1H;Started execution.\n");

    //free the framebuffer so sdl can use it instead
    //free(MEM_K1_TO_K0(__xfb));
    //__xfb = NULL;
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

    g_AnmManager = new AnmManager();

    if (GameWindow::InitD3dRendering() != ZUN_SUCCESS)
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
    if (!g_Supervisor.cfg.windowed)
    {
        SDL_ShowCursor(SDL_DISABLE);
    }

    g_GameWindow.curFrame = 0;

    while (true)
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
    g_Chain.Release();
    g_SoundPlayer.Release();

    delete g_AnmManager;
    g_AnmManager = NULL;

    if (g_GfxBackend != NULL)
        delete g_GfxBackend;
    SDL_Quit();

    if (renderResult == 2)
    {
        g_GameErrorContext.ResetContext();

        g_GameErrorContext.Log(TH_ERR_OPTION_CHANGED_RESTART);

        if (!g_Supervisor.cfg.windowed)
        {
            SDL_ShowCursor(SDL_ENABLE);
        }

        goto restart;
    }

    FileSystem::WriteDataToFile(TH_CONFIG_FILE, &g_Supervisor.cfg, sizeof(g_Supervisor.cfg));
    //    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, g_GameWindow.screenSaveActive, NULL, SPIF_SENDCHANGE);
    //    SystemParametersInfo(SPI_SETLOWPOWERACTIVE, g_GameWindow.lowPowerActive, NULL, SPIF_SENDCHANGE);
    //    SystemParametersInfo(SPI_SETPOWEROFFACTIVE, g_GameWindow.powerOffActive, NULL, SPIF_SENDCHANGE);

    SDL_ShowCursor(SDL_ENABLE);
    g_GameErrorContext.Flush();
    return 0;
}
