/*
Author:     Alexander Mundt 101632886
Class:      GAME2005
Professor:  Connor Smiley
Assignment: Lab Exercise 4
*/
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include <array>
#include <vector>
#include <cassert>

enum ColliderType
{
    COLLIDER_TYPE_NONE = 0,
    COLLIDER_TYPE_CIRCLE = 1,
    COLLIDER_TYPE_PLANE = 2
};

struct Collider
{
    ColliderType type;
    union
    {
        struct
        {
            float radius;
        };
        struct
        {
            Vector2 normal;
        };
    };
};

struct PhysicsBody
{
    Vector2 pos;
    Vector2 vel;
    Vector2 acc;

    float gravity_scale;
    float mass;
    float drag;

    Collider collider;
    bool collision;
};

constexpr Vector2 GRAVITY = { 0.0f, 100.0f };

bool HitTestCircles(Vector2 posA, float radiusA, Vector2 posB, float radiusB)
{
    Vector2 distanceAB = { posB.x - posA.x, posB.y - posA.y };
    float squaredDistance = distanceAB.x * distanceAB.x + distanceAB.y * distanceAB.y;
    float squaredRadiusSum = (radiusA + radiusB) * (radiusA + radiusB);
    return squaredDistance <= squaredRadiusSum;
}

bool HitTestCirclePlane(Vector2 posCircle, float radius, Vector2 posPlane, Vector2 normal)
{
    Vector2 planeToCircle = posCircle - posPlane;
    return Vector2DotProduct(planeToCircle, normal) <= radius;
}

void InitBodies(std::vector<PhysicsBody>* bodies);

int main()
{
    std::vector<PhysicsBody> bodies;

    //Initial plane (halfspace) and circle that we are going to test collision on
    InitBodies(&bodies);

    InitWindow(800, 800, "GAME-2005 Lab Exercise 4 - Alexander Mundt 101632886");
    InitAudioDevice();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        for (PhysicsBody& body : bodies)
        {
            body.acc = GRAVITY;
            body.vel += body.acc * body.gravity_scale * dt;
            body.vel *= powf(body.drag, dt);
            body.pos += body.vel * dt;

            body.collision = false;
        }

        for (size_t i = 0; i < bodies.size(); i++)
        {
            for (size_t j = i + 1; j < bodies.size(); j++)
            {
                PhysicsBody& a = bodies[i];
                PhysicsBody& b = bodies[j];

                bool collision = HitTestCirclePlane(a.pos, a.collider.radius, b.pos, b.collider.normal);
                a.collision |= collision;
                b.collision |= collision;
            }
        }

        // Hard-coding data to visualize circle-plane projection
        // In the future, we will make our nested collision loop handle all permutations of collision geometry arrangements (ie plane-circle vs circle-plane)
        PhysicsBody& plane = bodies[1];

        if (IsKeyDown(KEY_E))
        {
            plane.collider.normal = Vector2Rotate(plane.collider.normal, 100.0f * DEG2RAD * dt);

        }
        if (IsKeyDown(KEY_Q))
        {
            plane.collider.normal = Vector2Rotate(plane.collider.normal, -100.0f * DEG2RAD * dt);
        }

        Vector2 toCircle = GetMousePosition() - plane.pos;
        float scalarProj = Vector2DotProduct(toCircle, plane.collider.normal);
        Color testColor = scalarProj >= 20.0f ? BLUE : DARKBLUE; //20.0f because that is the radius of the circle we are going to draw up ahead
        Vector2 projectionCircle = plane.pos + plane.collider.normal * Vector2DotProduct(toCircle, plane.collider.normal);

        BeginDrawing();
        ClearBackground(WHITE);
        DrawFPS(720, 8);

        //Mouse circle
        DrawCircleV(GetMousePosition(), 20.0f, SKYBLUE);

        //Circle along the plane's normal
        DrawCircleV(projectionCircle, 20.0f, testColor);

        for (const PhysicsBody& body : bodies)
        {
            Color color = body.collision ? RED : GREEN;
            const Collider& collider = body.collider;

            if (collider.type == COLLIDER_TYPE_CIRCLE)
            {
                DrawCircleV(body.pos, collider.radius, color);
            }

            if (collider.type == COLLIDER_TYPE_PLANE)
            {
                //Perpendicular line
                Vector2 perp = { collider.normal.y, -collider.normal.x };
                DrawLineEx(body.pos - perp * 100.0f, body.pos + perp * 100.0f, 4.0f, color);

                //Plane normal line
                DrawLineEx(body.pos, body.pos + collider.normal * 50.0f, 2.0f, ORANGE);
            }
        }

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void InitBodies(std::vector<PhysicsBody>* bodies)
{
    {
        PhysicsBody circle{};

        circle.pos = { 400.0f, 400.0f };
        circle.vel = Vector2Zeros;

        circle.gravity_scale = 1.0f;
        circle.drag = 1.0f;

        circle.collider.type = COLLIDER_TYPE_CIRCLE;
        circle.collider.radius = 20.0f;

        bodies->push_back(circle);
    }
    {
        PhysicsBody plane{};

        plane.pos = { 400.0f, 600.0f };
        plane.vel = Vector2Zeros;

        plane.gravity_scale = 0.0f;
        plane.drag = 1.0f;

        plane.collider.type = COLLIDER_TYPE_PLANE;
        plane.collider.normal = Vector2UnitY * -1.0f;

        bodies->push_back(plane);
    }

    for (const PhysicsBody& body : *bodies)
    {
        //Ensure data is valid before first body update
        assert(body.collider.type != COLLIDER_TYPE_NONE);
    }
}