#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

int main()
{
    InitWindow(800, 800, "Physics-1");
    InitAudioDevice();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        float tt = GetTime();       //Total time - time since the window was initialized
        float dt = GetFrameTime();  //Frame time - 16.66 ms at 60 fps

        BeginDrawing();
        ClearBackground(WHITE);

        DrawFPS(720, 8);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
