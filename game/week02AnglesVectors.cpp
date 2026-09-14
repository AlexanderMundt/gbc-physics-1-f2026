#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

int main()
{
    InitWindow(800, 800, "Physics-1");
    InitAudioDevice();
    SetTargetFPS(60);

    Vector2 centre = { 400.0f, 400.0f };
    Vector2 start_direction = Vector2UnitX;
    float line_length = 200.0f;
    float degrees = 30.0f;

    Vector2 end_direction = Vector2Rotate(start_direction, -degrees * DEG2RAD);

    while (!WindowShouldClose())
    {
        DrawCircleV(centre, 10.0f, GRAY);
        DrawLineEx(centre, centre + start_direction * line_length, 4.0f, RED);
        DrawLineEx(centre, centre + end_direction * line_length, 4.0f, ORANGE);

        BeginDrawing();
        ClearBackground(WHITE);
        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}