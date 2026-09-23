#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include <array>
#include <vector>

struct PhysicsBody
{
    Vector2 position;
    // Add velocity, drag, and mass to complete LE2 step 1
// (Mass and drag will NOT be used in this lab exercise)
};

constexpr Vector2 GRAVITY = { 0.0f, -9.81f };

int main()
{
    InitWindow(800, 800, "Physics-1");
    InitAudioDevice();
    SetTargetFPS(60);

    Vector2 launchPos = { 100.0f, 700.0f };
    Vector2 launchDir = { 0.0f, 0.0f };
    Vector2 launchVel = { 0.0f, 0.0f };
    float launchAng = 0.0f;
    float launchSpd = 100.0f;

    while (!WindowShouldClose())
    {
        float tt = GetTime();       //Total time - time since the window was initialized
        float dt = GetFrameTime();  //Frame time - 16.66 ms at 60 fps

        launchDir = Vector2Rotate(Vector2UnitX, -launchAng * DEG2RAD);
        launchVel = launchDir * launchSpd;

        if (IsKeyPressed(KEY_SPACE))
        {
            // LE2 step 3 - Add the ability to instantiate a physics body and launch it from launch_position at launch_velocity
        }

        // Ensure motion is applied to physics bodies frame-after-frame using kinematics (position, velocity, and acceleration). Hint:
        // acc = gravity * dt
        // vel += acc * dt
        // pos += vel * dt

        BeginDrawing();
        ClearBackground(WHITE);
        DrawFPS(720, 8);
        
        DrawCircleV(launchPos, 20.0f, RED);
        DrawLineEx(launchPos, launchPos + launchVel, 5.0f, ORANGE);

        //GUI
        DrawText(TextFormat("Launch Angle %.1f", launchAng), 32.0f, 30.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 20.0f, 50.0f, 160.0f, 40.0f }, "0", "90", &launchAng, 0.0f, 90.0f);
        
        DrawText(TextFormat("Launch Speed %.1f", launchSpd), 32.0f, 100.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 20.0f, 120.0f, 160.0f, 40.0f }, "10", "300", &launchSpd, 10.0f, 300.0f);
        
        DrawText(TextFormat("Launch PosX %.0f", launchPos.x), 252.0f, 30.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 240.0f, 50.0f, 160.0f, 40.0f }, "0", "800", &launchPos.x, 0.0f, 800.0f);
        
        DrawText(TextFormat("Launch PosY %.0f", launchPos.y), 252.0f, 100.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 240.0f, 120.0f, 160.0f, 40.0f }, "0", "800", &launchPos.y, 0.0f, 800.0f);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
