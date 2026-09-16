/*
Author:     Alexander Mundt 101632886
Class:      GAME2005
Professor:  Connor Smiley
Assignment: Lab Exercise 1
*/
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

int main()
{
    InitWindow(800, 800, "Lab Exercise 1");
    InitAudioDevice();
    SetTargetFPS(60);

    Vector2 launchPos = { 100.0f, 700.0f };
    Vector2 launchDir = { 0.0f, 0.0f };
    Vector2 launchVel = { 0.0f, 0.0f };
    float launchAng = 0.0f;
    float launchSpd = 100.0f;

    //EX:
    //Note that since raylib is in RHS, positive rotations are CLOCKWISE
    //Negate the launch angle so that your launch direction vector points up
    Vector2 exPos = { 400.0f, 400.0f };
    Vector2 exDir = Vector2Rotate(Vector2UnitX, -30.0f * DEG2RAD);

    while (!WindowShouldClose())
    {
        //Calculate the launchDir Vector2 by using the Vector2Rotate function
        //Be sure to convert launchAng from degrees to radians when assinging it to Vector2Rotate

        //Calculate launchVel Vector2 by multiplying launchDir by launchSpd

        BeginDrawing();
        ClearBackground(WHITE);

        //Render launchVel as a line from launchPos to launchPos + launchVel
        DrawCircleV(launchPos, 20.0f, GRAY);

        //GUI
        DrawText(TextFormat("Launch Angle %.1f", launchAng), 32.0f, 30.0f, 16, DARKGRAY);
        DrawText(TextFormat("Launch Speed %.1f", launchSpd), 32.0f, 100.0f, 16, DARKGRAY);
        GuiSlider({ 20.0f, 50.0f, 160.0f, 40.0f }, "0", "90", &launchAng, 0.0f, 90.0f);
        GuiSlider({ 20.0f, 120.0f, 160.0f, 40.0f }, "10", "300", &launchSpd, 10.0f, 300.0f);

        //EX:
        //An illistration of a rotated vector
        DrawLineEx(exPos, exPos + exDir * 100.0f, 4.0f, ORANGE);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
