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
    Color color;
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

void UpdateMotion(PhysicsBody* body, float dt)
{
    body->acc = GRAVITY;
    body->vel += body->acc * body->gravity_scale * dt;
    body->vel *= powf(body->drag, dt);
    body->pos += body->vel * dt;
}

void DrawBody(PhysicsBody body, Color color)
{
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

class Scene
{
public:
    virtual void Load() {}
    virtual void Unload() {}

    virtual void Update(float dt) {}
    virtual void Draw() {}
};

enum SceneType : int
{
    SCENE_CIRCLES,
    SCENE_CIRCLE_PLANE,
    SCENE_BALL_PIT,
    SCENE_COUNT
};

class CircleTestScene : public Scene
{
public:
    void Load() final
    {
        circle1 = {};
        circle1.pos = { 400.0f, 400.0f };
        circle1.drag = 1.0f;
        circle1.collider.type = COLLIDER_TYPE_CIRCLE;
        circle1.collider.radius = 20.0f;

        circle2 = {};
        circle2.pos = { 400.0f, 200.0f };
        circle2.drag = 1.0f;
        circle2.collider.type = COLLIDER_TYPE_CIRCLE;
        circle2.collider.radius = 20.0f;
    }

    void Update(float dt) final
    {
        circle1.collision = circle2.collision = false;
        UpdateMotion(&circle1, dt);
        UpdateMotion(&circle2, dt);

        static bool use_mouse = true;
        if (IsKeyPressed(KEY_M)) use_mouse = !use_mouse;
        if (use_mouse) circle2.pos = GetMousePosition();

        // LE5: Should be able to push circle1 around with the mouse if done correctly!
        Vector2 mtv = Vector2Zeros;
        bool collision = HitTestCircles(circle1.pos, circle1.collider.radius, circle2.pos, circle2.collider.radius, &mtv);
        circle1.pos += mtv;
        circle1.collision |= collision;
        circle2.collision |= collision;
    }

    void Draw() final
    {
        DrawBody(circle1, circle1.collision ? RED : GREEN);
        DrawBody(circle2, circle2.collision ? RED : GREEN);
        DrawProjCircles(circle1, circle2);
    }

private:
    PhysicsBody circle1;
    PhysicsBody circle2;

    void DrawProjCircles(PhysicsBody a, PhysicsBody b)
    {
        float len_ab = Vector2Length(b.pos - a.pos);
        Vector2 dir_ab = Vector2Normalize(b.pos - a.pos);
        DrawLineEx(a.pos, b.pos, 4.0f, PURPLE);
        DrawLineEx(a.pos, a.pos + dir_ab * a.collider.radius, 3.0f, BLUE);
        DrawLineEx(b.pos, b.pos - dir_ab * b.collider.radius, 3.0f, BLUE);
    }
};

class CirclePlaneTestScene : public Scene
{
public:
    void Load() final
    {
        circle = {};
        circle.pos = { 400.0f, 400.0f };
        circle.gravity_scale = 1.0f;
        circle.drag = 1.0f;
        circle.collider.type = COLLIDER_TYPE_CIRCLE;
        circle.collider.radius = 20.0f;

        plane = {};
        plane.pos = { 400.0f, 600.0f };
        plane.collider.type = COLLIDER_TYPE_PLANE;
        plane.collider.normal = Vector2Rotate(Vector2UnitX, -45.0F * DEG2RAD);
    }

    void Update(float dt) final
    {
        circle.collision = plane.collision = false;
        UpdateMotion(&circle, dt);
        UpdateMotion(&plane, dt);

        if (IsKeyDown(KEY_E))
            plane.collider.normal = Vector2Rotate(plane.collider.normal, 100.0f * DEG2RAD * dt);
        if (IsKeyDown(KEY_Q))
            plane.collider.normal = Vector2Rotate(plane.collider.normal, -100.0f * DEG2RAD * dt);

        Vector2 mtv = Vector2Zeros;
        bool collision = HitTestCirclePlane(circle.pos, circle.collider.radius, plane.pos, plane.collider.normal, &mtv);
        circle.pos += mtv;
        circle.collision |= collision;
        plane.collision |= collision;
    }

    void Draw() final
    {
        DrawBody(circle, circle.collision ? RED : GREEN);
        DrawBody(plane, plane.collision ? RED : GREEN);
        DrawProjCirclePlane(circle, plane);
    }

private:
    PhysicsBody circle;
    PhysicsBody plane;

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
};

class BallPitScene : public Scene
{
public:
    void Load() final
    {
        {
            PhysicsBody wall_left{};
            wall_left.pos = { 0.0f, 400.0f };
            wall_left.collider.type = COLLIDER_TYPE_PLANE;
            wall_left.collider.normal = Vector2UnitX;
            bodies.push_back(wall_left);
        }

        {
            PhysicsBody wall_right{};
            wall_right.pos = { 800.0f, 400.0f };
            wall_right.collider.type = COLLIDER_TYPE_PLANE;
            wall_right.collider.normal = Vector2UnitX * -1.0f;
            bodies.push_back(wall_right);
        }

        {
            PhysicsBody wall_top{};
            wall_top.pos = { 400.0f, 0.0f };
            wall_top.collider.type = COLLIDER_TYPE_PLANE;
            wall_top.collider.normal = Vector2UnitY;
            bodies.push_back(wall_top);
        }

        {
            PhysicsBody wall_bottom{};
            wall_bottom.pos = { 400.0f, 800.0f };
            wall_bottom.collider.type = COLLIDER_TYPE_PLANE;
            wall_bottom.collider.normal = Vector2UnitY * -1.0f;
            bodies.push_back(wall_bottom);
        }
    }

    void Unload() final
    {
        bodies.clear();
    }

    void Update(float dt) final
    {
        const float time_total = 0.5f;
        static float time = 0.0f;
        time += dt;
        if (time >= time_total)
        {
            time = 0.0f;

            PhysicsBody circle{};
            circle.pos = { 400.0f, 400.0f };
            circle.vel = Vector2Rotate(Vector2UnitY * -1.0f, GetRandomValue(-60, 60) * DEG2RAD) * 100.0f;
            circle.gravity_scale = 1.0f;
            circle.drag = 1.0f;
            circle.collider.type = COLLIDER_TYPE_CIRCLE;
            circle.collider.radius = 20.0f;
            circle.color.r = GetRandomValue(0, 255);
            circle.color.g = GetRandomValue(0, 255);
            circle.color.b = GetRandomValue(0, 255);
            circle.color.a = 255;
            bodies.push_back(circle);
        }

        for (PhysicsBody& body : bodies)
        {
            body.collision = false;
            UpdateMotion(&body, dt);
        }

        // TODO for lab -- make collision loop nice
        // (Note that some balls get stuck in walls because LE5 is not yet applying the MTV)
        for (size_t i = 0; i < bodies.size(); i++)
        {
            for (size_t j = i; j < bodies.size(); j++)
            {
                // Extra practice: change ball colour whenever it hits a wall!
                PhysicsBody& a = bodies[i];
                PhysicsBody& b = bodies[j];

                if (a.collider.type == COLLIDER_TYPE_CIRCLE && b.collider.type == COLLIDER_TYPE_PLANE)
                {
                    bool collision = HitTestCirclePlane(a.pos, a.collider.radius, b.pos, b.collider.normal);
                    if (collision) a.vel = Vector2Reflect(a.vel, b.collider.normal);
                }
                if (a.collider.type == COLLIDER_TYPE_PLANE && b.collider.type == COLLIDER_TYPE_CIRCLE)
                {
                    bool collision = HitTestCirclePlane(b.pos, b.collider.radius, a.pos, a.collider.normal);
                    if (collision) b.vel = Vector2Reflect(b.vel, a.collider.normal);
                }
            }
        }

        // We could also do this a more naive way where we flip velocity direction based on the screen edge we're hitting,
        // but we're physics programmers, therefore we used our plane colliders!!!
        //for (PhysicsBody& body : bodies)
        //{
        //
        //}
    }

    void Draw() final
    {
        for (PhysicsBody& body : bodies)
        {
            DrawBody(body, body.color);
        }
    }

private:
    std::vector<PhysicsBody> bodies;
};

int main()
{
    InitWindow(800, 800, "Physics-1");
    InitAudioDevice();
    SetTargetFPS(60);

    const char* scene_names[SCENE_COUNT];
    scene_names[SCENE_CIRCLES] = "Circle-Circle Test";
    scene_names[SCENE_CIRCLE_PLANE] = "Circle-Plane Test";
    scene_names[SCENE_BALL_PIT] = "Ball Pit";

    Scene* scenes[SCENE_COUNT];
    scenes[SCENE_CIRCLES] = new CircleTestScene;
    scenes[SCENE_CIRCLE_PLANE] = new CirclePlaneTestScene;
    scenes[SCENE_BALL_PIT] = new BallPitScene;

    int scene_index = SCENE_COUNT - 1;
    scenes[scene_index]->Load();
    SetWindowTitle(scene_names[scene_index]);

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_TAB))
        {
            scenes[scene_index]->Unload();
            ++scene_index %= SCENE_COUNT;
            scenes[scene_index]->Load();
            SetWindowTitle(scene_names[scene_index]);
        }

        float dt = GetFrameTime();
        scenes[scene_index]->Update(dt);

        BeginDrawing();
        ClearBackground(WHITE);
        DrawFPS(720, 10);
        scenes[scene_index]->Draw();
        EndDrawing();
    }

    scenes[scene_index]->Unload();
    for (int i = 0; i < SCENE_COUNT; i++)
        delete scenes[i];

    CloseAudioDevice();
    CloseWindow();
    return 0;
}