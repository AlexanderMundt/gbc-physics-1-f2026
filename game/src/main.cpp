#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

int main()
{
    InitWindow(800, 800, "Alexander Mundt");
    InitAudioDevice();
    SetTargetFPS(60);

    float x = 400.0f;
    float y = 400.0f;
    float x2 = 500.0f;
    float y2 = 400.0f;
    float a = 30.0f; //Frequency
    float b = 0.05f; //Amplitude

    Vector2 pos = { 100.0f, 400.0f };

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        float t = GetTime();

        //Simple Animation
        pos += Vector2UnitX * 100.0f * dt;

        BeginDrawing();
        ClearBackground(WHITE);

        DrawCircleV(pos, 20.0f, RED);

        DrawCircle(50, 100, 25.0f, GREEN);
        DrawCircle(150, 250, 25.0f, GREEN);
        DrawLine(50, 100, 150, 250, LIME);

        DrawCircleV(GetMousePosition(), 20.0f, RED);
        DrawText("Alexander Mundt 10163288", 495, 755, 20, BLUE);
        //int x = MeasureText("Alexander Mundt 10163288", 20);
        
        //Use TextFormat to convert data like int or float to const char*
        //TODO: replace with time
        //float a = 42.123f;
        //int b = 5;
        const char* text = TextFormat("DeltaTime dt: %f, Time t: %f.", dt, t);
        DrawText(text, 200, 600, 20, BLUE);

        //TODO: animate this based on equation in lab doc week 1
        y += (cos(t * a)) * a * b * dt;
        x += (-sin(t * a)) * a * b * dt;
        DrawCircle(x, y, 25.0f, PURPLE);

        y2 += (cos(t * a)) * a * b * t;
        x2 += (-sin(t * a)) * a * b * t;
        DrawCircle(x2, y2, 25.0f, PURPLE);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
