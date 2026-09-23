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
        BeginDrawing();
        ClearBackground(WHITE);

        //WEEK3

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
