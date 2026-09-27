/*
Author:     Alexander Mundt 101632886
Class:      GAME2005
Professor:  Connor Smiley
Assignment: Lab Exercise 2
*/
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include <vector>

//1.a b c d
struct PhysicsBody
{
    Vector2 position;
    Vector2 velocity;
    Vector2 drag;
    float mass;

    //For 5.
    std::vector<Vector2>* pathToPlot;

    PhysicsBody()
    {
        position = Vector2Zeros;
        velocity = Vector2Zeros;
        drag = Vector2Zeros;
        mass = 0.0f;

        pathToPlot = new std::vector<Vector2>();
        pathToPlot->push_back(position);
    }

    PhysicsBody(Vector2 pos, Vector2 vel)
    {
        position = pos;
        velocity = vel;
        drag = Vector2Zeros;
        mass = 0.0f;

        pathToPlot = new std::vector<Vector2>();
        pathToPlot->push_back(position);
    }

    ~PhysicsBody()
    {
        delete pathToPlot;
    }
};

//Constant
constexpr float GRAVITY = -9.81f;
constexpr float COSMETIC_LINE_LENGTHENER = 10.0f;

int main()
{
    InitWindow(800, 800, "Physics-1");
    InitAudioDevice();
    SetTargetFPS(60);

    Vector2 launchPos = { 100.0f, 700.0f };
    Vector2 launchDir = Vector2Zeros;
    Vector2 launchVel = Vector2Zeros;
    float launchAng = 0.0f;
    float launchSpd = 5.0f;

    //For the launched projectile
    PhysicsBody* pb = nullptr;

    //2.b a
    float tt = 0.0f;
    float dt = 0.0f;

    //2.c
    Vector2 gravity = { 0.0f, GRAVITY };

    while (!WindowShouldClose())
    {
        //2.b a
        tt = GetTime();       //Total time - time since the window was initialized
        dt = GetFrameTime();  //Frame time - 16.66ms / 0.016s at 60 fps

        launchDir = Vector2Rotate(Vector2UnitX, -launchAng * DEG2RAD);
        launchVel = launchDir * launchSpd;

        if (IsKeyPressed(KEY_SPACE))
        {
            //Clean up
            if (pb != nullptr)
            {
                delete pb;
            }
            pb = new PhysicsBody(launchPos, launchVel);
        }

        //3.
        if (pb != nullptr)
        {
            //Use negate here to make sure gravity makes the projectile go DOWN on the screen
            pb->velocity += Vector2Scale(Vector2Negate(gravity), dt);
            pb->position += pb->velocity;
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawFPS(720, 8);
        
        //Launch point circle and projection lines
        DrawCircleV(launchPos, 20.0f, RED);
        DrawLineEx(launchPos, launchPos + launchVel, 5.0f, ORANGE);
        DrawLineEx(launchPos, launchPos + launchVel * COSMETIC_LINE_LENGTHENER, 2.0f, PINK);
        
        //3.
        if (pb != nullptr)
        {
            //Draw the projectile at the new calculated position
            DrawCircleV(pb->position, 15.0f, BLUE);

            //Add that position to the vector of points that we will draw a line on
            pb->pathToPlot->push_back(pb->position);

            //5.
            //Plot the path that the projectile has traveled 
            for (int i = 0, j = 1; j < pb->pathToPlot->size(); i++, j++)
            {
                //From previous pos(i) to current pos(j)
                DrawLineEx(pb->pathToPlot->at(i), pb->pathToPlot->at(j), 1.5f, SKYBLUE);
            }
        }

        //GUI
        DrawText(TextFormat("Launch Angle %.1f", launchAng), 32.0f, 30.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 20.0f, 50.0f, 160.0f, 40.0f }, "0", "90", &launchAng, 0.0f, 90.0f);
        
        DrawText(TextFormat("Launch Speed %.1f", launchSpd), 32.0f, 100.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 20.0f, 120.0f, 160.0f, 40.0f }, "5", "20", &launchSpd, 5.0f, 20.0f);
        
        DrawText(TextFormat("Launch PosX %.0f", launchPos.x), 252.0f, 30.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 240.0f, 50.0f, 160.0f, 40.0f }, "0", "800", &launchPos.x, 0.0f, 800.0f);
        
        DrawText(TextFormat("Launch PosY %.0f", launchPos.y), 252.0f, 100.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 240.0f, 120.0f, 160.0f, 40.0f }, "0", "800", &launchPos.y, 0.0f, 800.0f);

        //4.
        DrawText(TextFormat("Gravity %.2f", gravity.y), 472.0f, 30.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 460.0f, 50.0f, 160.0f, 40.0f }, "-20", "20", &gravity.y, -20.0f, 20.0f);

        DrawText(TextFormat("Note: The smaller orange line represents the actual velocity\n\t\tand the"
            " pink line is to help visuallize the amount of force\nCosmetic Multiplier: %.2f", 
            COSMETIC_LINE_LENGTHENER), 10.0f, 740.0f, 14, BLACK);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}