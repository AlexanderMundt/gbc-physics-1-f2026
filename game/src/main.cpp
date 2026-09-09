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

        DrawCircle(50, 100, 25.0f, GREEN);
        DrawCircle(150, 250, 25.0f, GREEN);
        DrawLine(50, 100, 150, 250, LIME);
        DrawCircleV(GetMousePosition(), 20.0f, RED);
        //int x = MeasureText("Alexander Mundt 10163288", 20);
        DrawText("Alexander Mundt 10163288", 495, 755, 20, BLUE);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
