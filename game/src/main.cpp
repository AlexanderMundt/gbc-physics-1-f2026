#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

int main()
{
    InitWindow(800, 800, "Alexander Mundt");
    InitAudioDevice();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);

        DrawCircleV(GetMousePosition(), 20.0f, RED);
        DrawText("Alexander Mundt 10163288", 650, 750, 20, BLUE);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
