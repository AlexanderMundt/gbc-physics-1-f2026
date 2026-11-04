/*
Author:     Alexander Mundt 101632886
Class:      GAME2005
Professor:  Connor Smiley
Assignment: Lab Exercise 5
*/
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include <array>
#include <vector>
#include <cassert>

enum TestType : int
{
    TEST_CIRCLES,
    TEST_CIRCLE_PLANE
};

enum ColliderType
{
    COLLIDER_TYPE_NONE,
    COLLIDER_TYPE_CIRCLE,
    COLLIDER_TYPE_PLANE
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

// MTV resolves A from B
bool HitTestCircles(Vector2 pos_a, float radius_a, Vector2 pos_b, float radius_b, Vector2* mtv = nullptr)
{
    bool collision = CheckCollisionCircles(pos_a, radius_a, pos_b, radius_b);
    if (collision && mtv != nullptr)
    {
        // 1) compute mtv magnitude by subtracting the distance between circles from the sum of their radii
        // 2) ensure mtv direction points *from* B *to* A so the mtv resolves A *from* B
        Vector2 direction = Vector2UnitY;
        float magnitude = 1.0f;
        *mtv = direction * magnitude;
    }

    return collision;
}

// MTV resolves A (circle) from B (plane)
bool HitTestCirclePlane(Vector2 pos_circle, float radius, Vector2 pos_plane, Vector2 normal, Vector2* mtv = nullptr)
{
    Vector2 to_circle = pos_circle - pos_plane;
    float scalar_proj = Vector2DotProduct(to_circle, normal);
    bool collision = scalar_proj <= radius;

    // mtv magnitude is length of projection of circle along plane normal (the scalar_proj variable)
    // mtv direction is plane normal
    if (collision && mtv != nullptr)
    {
        *mtv = Vector2UnitY * -10.0f;
    }

    return collision;
}

void DrawBody(PhysicsBody body);
void DrawProjCircles(PhysicsBody a, PhysicsBody b);
void DrawProjCirclePlane(PhysicsBody circle, PhysicsBody plane);

void InitBodiesCircles(std::vector<PhysicsBody>* bodies);
void InitBodiesCirclePlane(std::vector<PhysicsBody>* bodies);

int main()
{
    InitWindow(800, 800, "Physics-1");
    InitAudioDevice();
    SetTargetFPS(60);

    int test = TEST_CIRCLES;
    std::vector<PhysicsBody> bodies;
    InitBodiesCircles(&bodies);

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

        if (IsKeyPressed(KEY_T))
        {
            ++test %= 2;
            bodies.clear();
            if (test == TEST_CIRCLES)
                InitBodiesCircles(&bodies);
            else if (test == TEST_CIRCLE_PLANE)
                InitBodiesCirclePlane(&bodies);
        }

        PhysicsBody& circle1 = bodies[0];
        switch (test)
        {
        case TEST_CIRCLES:
        {
            PhysicsBody& circle2 = bodies[1];
            static bool use_mouse = true;
            if (IsKeyPressed(KEY_TAB)) use_mouse = !use_mouse;
            if (use_mouse) circle2.pos = GetMousePosition();

            // LE5: Should be able to push circle1 around with the mouse if done correctly!
            Vector2 mtv = Vector2Zeros;
            bool collision = HitTestCircles(circle1.pos, circle1.collider.radius, circle2.pos, circle2.collider.radius, &mtv);
            circle1.pos += mtv;
        }
        break;

        case TEST_CIRCLE_PLANE:
        {
            PhysicsBody& plane = bodies[1];
            if (IsKeyDown(KEY_E))
                plane.collider.normal = Vector2Rotate(plane.collider.normal, 100.0f * DEG2RAD * dt);
            if (IsKeyDown(KEY_Q))
                plane.collider.normal = Vector2Rotate(plane.collider.normal, -100.0f * DEG2RAD * dt);

            // LE5: circle1 should roll off the plane if done correctly!
            Vector2 mtv = Vector2Zeros;
            bool collision = HitTestCirclePlane(circle1.pos, circle1.collider.radius, plane.pos, plane.collider.normal, &mtv);
            circle1.pos += mtv;
        }
        break;
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawFPS(720, 10);

        for (const PhysicsBody& body : bodies)
        {
            DrawBody(body);
        }

        if (test == TEST_CIRCLES)
        {
            DrawProjCircles(bodies[0], bodies[1]);
        }
        else
        {
            DrawProjCirclePlane(bodies[0], bodies[1]);
        }

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void DrawBody(PhysicsBody body)
{
    Color color = body.collision ? RED : GREEN;
    const Collider& collider = body.collider;
    if (collider.type == COLLIDER_TYPE_CIRCLE)
    {
        DrawCircleV(body.pos, collider.radius, color);
    }
    if (collider.type == COLLIDER_TYPE_PLANE)
    {
        Vector2 perp = { collider.normal.y, -collider.normal.x };
        DrawLineEx(body.pos - perp * 1000.0f, body.pos + perp * 1000.0f, 4.0f, color);
        DrawLineEx(body.pos, body.pos + collider.normal * 50.0f, 2.0f, ORANGE);
    }
}

void DrawProjCircles(PhysicsBody a, PhysicsBody b)
{
    float len_ab = Vector2Length(b.pos - a.pos);
    Vector2 dir_ab = Vector2Normalize(b.pos - a.pos);
    DrawLineEx(a.pos, b.pos, 4.0f, PURPLE);
    DrawLineEx(a.pos, a.pos + dir_ab * a.collider.radius, 3.0f, BLUE);
    DrawLineEx(b.pos, b.pos - dir_ab * b.collider.radius, 3.0f, BLUE);
}

void DrawProjCirclePlane(PhysicsBody circle, PhysicsBody plane)
{
    Vector2 to_circle = circle.pos - plane.pos;
    float scalar_proj = Vector2DotProduct(to_circle, plane.collider.normal);
    bool collision = scalar_proj <= circle.collider.radius;
    Color test_color = collision ? RED : GREEN;
    Vector2 proj = plane.pos + plane.collider.normal * scalar_proj;
    DrawLineEx(plane.pos, proj, 4.0f, PURPLE);
    DrawLineEx(circle.pos, proj, 2.0f, VIOLET);
    DrawCircleV(proj, 5.0f, ORANGE);
}

void InitBodiesCircles(std::vector<PhysicsBody>* bodies)
{
    {
        PhysicsBody circle{};
        circle.pos = { 400.0f, 400.0f };
        circle.drag = 1.0f;
        circle.collider.type = COLLIDER_TYPE_CIRCLE;
        circle.collider.radius = 20.0f;
        bodies->push_back(circle);
    }

    {
        PhysicsBody circle{};
        circle.pos = { 400.0f, 200.0f };
        circle.drag = 1.0f;
        circle.collider.type = COLLIDER_TYPE_CIRCLE;
        circle.collider.radius = 20.0f;
        bodies->push_back(circle);
    }

    for (const PhysicsBody& body : *bodies)
    {
        // Ensure data is valid before first body update
        assert(body.collider.type != COLLIDER_TYPE_NONE);
    }
}

void InitBodiesCirclePlane(std::vector<PhysicsBody>* bodies)
{
    {
        PhysicsBody circle{};
        circle.pos = { 400.0f, 400.0f };
        circle.gravity_scale = 1.0f;
        circle.drag = 1.0f;
        circle.collider.type = COLLIDER_TYPE_CIRCLE;
        circle.collider.radius = 20.0f;
        bodies->push_back(circle);
    }

    {
        PhysicsBody plane{};
        plane.pos = { 400.0f, 600.0f };
        plane.collider.type = COLLIDER_TYPE_PLANE;
        plane.collider.normal = Vector2Rotate(Vector2UnitX, -45.0F * DEG2RAD);
        bodies->push_back(plane);
    }

    for (const PhysicsBody& body : *bodies)
    {
        // Ensure data is valid before first body update
        assert(body.collider.type != COLLIDER_TYPE_NONE);
    }
}