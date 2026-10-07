/*
Author:     Alexander Mundt 101632886
Class:      GAME2005
Professor:  Connor Smiley
Assignment: Lab Exercise 2

EDITED ON: OCT 7th 2026
*/
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include <vector>
#include <cmath>
#include <cassert>

enum ColliderType
{
    COLLIDER_TYPE_NONE = 0,
    COLLIDER_TYPE_CIRCLE = 1,
};

struct Collider
{
    ColliderType type;
    union
    {
        //Cirlce collider
        struct
        {
            float radius;
        };
        ////Box collider
        //struct
        //{
        //    float width;
        //    float height;
        //};
    };
};

//1.a b c d
struct PhysicsBody
{
    Vector2 position;
    Vector2 velocity;
    Vector2 accel;

    float gravityScale;
    float drag;
    float mass;

    Collider collider;
    bool collision;

    //For 5.
    std::vector<Vector2> pathToPlot;

    PhysicsBody(Vector2 pos, Vector2 vel)
    {
        position = pos;
        velocity = vel;
        accel = Vector2Zeros;

        gravityScale = 0.0f;
        drag = 1.0f;
        mass = 1.0f;

        collider = Collider();
        collision = false;

        pathToPlot = std::vector<Vector2>();
        pathToPlot.push_back(position);
    }
};

bool HitTestCircles(Vector2 posA, float radiusA, Vector2 posB, float radiusB)
{
    //Lab Exercise 3 TODO -- complete this function, VERY similar to raylib's CheckCollisionCircles function
    return false;
}

//Constant
constexpr float GRAVITY = 9.81f;

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
    float launchDrag = 1.0f;
    float startMass = 1.0f;

    //For the launched projectiles
    std::vector<PhysicsBody> bodies;
    {
        PhysicsBody body = PhysicsBody(Vector2(400.0f, 400.0f), Vector2Zeros);
        body.collider.type = COLLIDER_TYPE_CIRCLE;
        body.collider.radius = 20.0f;
        bodies.push_back(body);
    }

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
            PhysicsBody body = PhysicsBody(launchPos, launchVel);
            body.accel = gravity;

            body.gravityScale = 1.0f;
            body.drag = launchDrag;
            body.mass = startMass;

            body.collider.type = COLLIDER_TYPE_CIRCLE;
            body.collider.radius = 20.0f;

            bodies.push_back(body);
        }

        //3.
        for (PhysicsBody& b : bodies)
        {
            b.velocity += b.accel * b.gravityScale * dt;
            b.velocity *= powf(b.drag, dt);
            b.position += Vector2Scale(b.velocity, dt);
            
            b.collision = false;
        }

        //Checks all UNIQUE pairings
        for (size_t i = 0; i < bodies.size(); i++)
        {
            for (size_t j = i + 1; j < bodies.size(); j++)
            {
                PhysicsBody& a = bodies[i];
                PhysicsBody& b = bodies[j];
                //Lab Exercise 3 TODO -- Replace CheckCollisionCircles with HitTestCircles
                //bool collision = CheckCollisionCircles(a.position, a.collider.radius, b.position, b.collider.radius);
                bool collision = HitTestCircles(a.position, a.collider.radius, b.position, b.collider.radius);
                a.collision |= collision;
                b.collision |= collision;
            }
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawFPS(720, 8);
        
        //Launch point circle and projection lines
        DrawCircleV(launchPos, 20.0f, RED);
        DrawLineEx(launchPos, launchPos + launchVel, 5.0f, ORANGE);
        
        //3.
        for (PhysicsBody& b : bodies)
        {
            assert(b.collider.type != COLLIDER_TYPE_NONE);
            if (b.collider.type == COLLIDER_TYPE_CIRCLE)
            {
                Color c = b.collision ? RED : GREEN;
                //Draw the projectile at the new calculated position
                DrawCircleV(b.position, b.collider.radius, c);
            }

            //Add that position to the vector of points that we will draw a line on
            b.pathToPlot.push_back(b.position);

            //5.
            //Plot the path that the projectile has traveled 
            for (int i = 0, j = 1; j < b.pathToPlot.size(); i++, j++)
            {
                //From previous pos(i) to current pos(j)
                DrawLineEx(b.pathToPlot.at(i), b.pathToPlot.at(j), 1.5f, SKYBLUE);
            }
        }

        //GUI
        DrawText(TextFormat("Launch Angle %.1f", launchAng), 32.0f, 30.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 20.0f, 50.0f, 160.0f, 40.0f }, "0", "90", &launchAng, 0.0f, 90.0f);
        
        DrawText(TextFormat("Launch Speed %.1f", launchSpd), 32.0f, 100.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 20.0f, 120.0f, 160.0f, 40.0f }, "20", "200", &launchSpd, 20.0f, 200.0f);
        
        DrawText(TextFormat("Launch PosX %.0f", launchPos.x), 252.0f, 30.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 240.0f, 50.0f, 160.0f, 40.0f }, "0", "800", &launchPos.x, 0.0f, 800.0f);
        
        DrawText(TextFormat("Launch PosY %.0f", launchPos.y), 252.0f, 100.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 240.0f, 120.0f, 160.0f, 40.0f }, "0", "800", &launchPos.y, 0.0f, 800.0f);

        //4.
        DrawText(TextFormat("Gravity %.2f", gravity.y), 472.0f, 30.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 460.0f, 50.0f, 160.0f, 40.0f }, "-250", "250", &gravity.y, -250.0f, 250.0f);

        DrawText(TextFormat("Drag %.1f", launchDrag), 472.0f, 100.0f, 16, DARKGRAY);
        GuiSlider(Rectangle{ 460.0f, 120.0f, 160.0f, 40.0f }, "0.0", "1.0", &launchDrag, 0.0f, 1.0f);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}