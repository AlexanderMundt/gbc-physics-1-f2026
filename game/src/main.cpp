/*
Author:     Alexander Mundt 101632886
Class:      GAME2005
Professor:  Connor Smiley
Assignment: Lab Exercise 6
*/
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include <array>
#include <vector>
#include <cassert>

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

constexpr Vector2 GRAVITY = { 0.0f, 100.0f };

struct PhysicsBody
{
    Vector2 pos;
    Vector2 vel;
    Vector2 force;

    float gravity_scale;
    float mass_inverse;
    float drag;

    float coeff_friction;
    //float coeff_restitution; <-- LE7

    Collider collider;
    bool collision;
    Color color;
};

inline float BodyMass(PhysicsBody body)
{
    float mass = FloatEquals(body.mass_inverse, 0.0f) ? 0.0f : 1.0f / body.mass_inverse;
    return mass;
}

inline Vector2 BodyAcc(PhysicsBody body)
{
    Vector2 acc = body.force * body.mass_inverse;
    return acc;
}

inline Vector2 Vector2Tangent(Vector2 v, Vector2 normal)
{
    float proj = Vector2DotProduct(v, normal);
    Vector2 tangent = v - normal * proj;
    return tangent;
}

// MTV resolves A from B
bool HitTestCircles(Vector2 pos_a, float radius_a, Vector2 pos_b, float radius_b, Vector2* mtv = nullptr)
{
    bool collision = CheckCollisionCircles(pos_a, radius_a, pos_b, radius_b);
    if (collision && mtv != nullptr)
    {
        Vector2 direction = Vector2Normalize(pos_a - pos_b);
        float magnitude = (radius_a + radius_b) - Vector2Distance(pos_a, pos_b);
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

    if (collision && mtv != nullptr)
    {
        *mtv = normal * (radius - scalar_proj);
    }

    return collision;
}

void UpdateMotion(PhysicsBody* body, float dt)
{
    Vector2 acc = body->force * body->mass_inverse;
    acc += GRAVITY * body->gravity_scale;

    body->vel += acc * dt;
    body->vel *= powf(body->drag, dt);
    body->pos += body->vel * dt;

    body->force = Vector2Zeros;
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
    SCENE_FRICTION,
    //SCENE_IMPULSE,    <-- Lab Exercise 7 
    //SCENE_AABB,       <-- Lab Exercise 8
    SCENE_COUNT
};

class CircleTestScene : public Scene
{
public:
    void Load() final
    {
        circle1 = {};
        circle1.pos = { 400.0f, 400.0f };
        circle1.mass_inverse = 1.0f;
        circle1.drag = 1.0f;
        circle1.collider.type = COLLIDER_TYPE_CIRCLE;
        circle1.collider.radius = 20.0f;

        circle2 = {};
        circle2.pos = { 400.0f, 600.0f };
        circle2.mass_inverse = 1.0f;
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
        Vector2 launch_force = Vector2UnitY * -10000.0f;

        circle_light = {};
        circle_light.pos = { 200.0f, 400.0f };
        circle_light.mass_inverse = 1.0f / 1.0f;
        circle_light.gravity_scale = 1.0f;
        circle_light.drag = 1.0f;
        circle_light.collider.type = COLLIDER_TYPE_CIRCLE;
        circle_light.collider.radius = 20.0f;

        circle_medium = {};
        circle_medium.pos = { 400.0f, 400.0f };
        circle_medium.mass_inverse = 1.0f / 2.0f;
        circle_medium.gravity_scale = 1.0f;
        circle_medium.drag = 1.0f;
        circle_medium.collider.type = COLLIDER_TYPE_CIRCLE;
        circle_medium.collider.radius = 20.0f;

        circle_heavy = {};
        circle_heavy.pos = { 600.0f, 400.0f };
        circle_heavy.mass_inverse = 1.0f / 5.0f;
        circle_heavy.gravity_scale = 1.0f;
        circle_heavy.drag = 1.0f;
        circle_heavy.collider.type = COLLIDER_TYPE_CIRCLE;
        circle_heavy.collider.radius = 20.0f;

        circle_light.force += launch_force;
        circle_medium.force += launch_force;
        circle_heavy.force += launch_force;

        plane = {};
        plane.pos = { 400.0f, 600.0f };
        plane.collider.type = COLLIDER_TYPE_PLANE;
        plane.collider.normal = Vector2Rotate(Vector2UnitX, -75.0F * DEG2RAD);
    }

    void Update(float dt) final
    {
        circle_light.collision = circle_medium.collision = circle_heavy.collision = plane.collision = false;
        UpdateMotion(&circle_light, dt);
        UpdateMotion(&circle_medium, dt);
        UpdateMotion(&circle_heavy, dt);
        UpdateMotion(&plane, dt);

        if (IsKeyDown(KEY_E))
            plane.collider.normal = Vector2Rotate(plane.collider.normal, 100.0f * DEG2RAD * dt);
        if (IsKeyDown(KEY_Q))
            plane.collider.normal = Vector2Rotate(plane.collider.normal, -100.0f * DEG2RAD * dt);

        Vector2 mtv_light = Vector2Zeros;
        Vector2 mtv_medium = Vector2Zeros;
        Vector2 mtv_heavy = Vector2Zeros;
        bool collision_light = HitTestCirclePlane(circle_light.pos, circle_light.collider.radius, plane.pos, plane.collider.normal, &mtv_light);
        bool collision_medium = HitTestCirclePlane(circle_medium.pos, circle_medium.collider.radius, plane.pos, plane.collider.normal, &mtv_medium);
        bool collision_heavy = HitTestCirclePlane(circle_heavy.pos, circle_heavy.collider.radius, plane.pos, plane.collider.normal, &mtv_heavy);

        circle_light.pos += mtv_light;
        circle_medium.pos += mtv_medium;
        circle_heavy.pos += mtv_heavy;

        circle_light.collision |= collision_light;
        circle_medium.collision |= collision_medium;
        circle_heavy.collision |= collision_heavy;

        plane.collision |= collision_light || collision_medium || collision_heavy;
    }

    void Draw() final
    {
        DrawBody(circle_light, circle_light.collision ? RED : GREEN);
        DrawBody(circle_medium, circle_medium.collision ? RED : GREEN);
        DrawBody(circle_heavy, circle_heavy.collision ? RED : GREEN);
        DrawBody(plane, plane.collision ? RED : GREEN);
    }

private:
    PhysicsBody circle_light;
    PhysicsBody circle_medium;
    PhysicsBody circle_heavy;
    PhysicsBody plane;
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

            PhysicsBody circle_light{};
            circle_light.pos = { 400.0f, 400.0f };
            circle_light.vel = Vector2Rotate(Vector2UnitY * -1.0f, GetRandomValue(-60, 60) * DEG2RAD) * 100.0f;
            circle_light.mass_inverse = 1.0f;
            circle_light.gravity_scale = 1.0f;
            circle_light.drag = 1.0f;
            circle_light.collider.type = COLLIDER_TYPE_CIRCLE;
            circle_light.collider.radius = 20.0f;
            circle_light.color.r = GetRandomValue(0, 255);
            circle_light.color.g = GetRandomValue(0, 255);
            circle_light.color.b = GetRandomValue(0, 255);
            circle_light.color.a = 255;
            bodies.push_back(circle_light);
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
                PhysicsBody* a = &bodies[i];
                PhysicsBody* b = &bodies[j];

                // No point in testing for collision between two static bodies
                if (FloatEquals(a->mass_inverse, 0.0f), FloatEquals(b->mass_inverse, 0.0f)) continue;

                // Demo-specific hack so we don't deal with circle-circle collision
                if (a->collider.type == COLLIDER_TYPE_CIRCLE && b->collider.type == COLLIDER_TYPE_CIRCLE) continue;

                Vector2 mtv = Vector2Zeros;
                bool collision = false;
                if (a->collider.type == COLLIDER_TYPE_CIRCLE && b->collider.type == COLLIDER_TYPE_PLANE)
                {
                    collision = HitTestCirclePlane(a->pos, a->collider.radius, b->pos, b->collider.normal, &mtv);
                }
                if (a->collider.type == COLLIDER_TYPE_PLANE && b->collider.type == COLLIDER_TYPE_CIRCLE)
                {
                    collision = HitTestCirclePlane(b->pos, b->collider.radius, a->pos, a->collider.normal, &mtv);
                }
                a->collision = b->collision |= collision;

                // Swap A and B if A is static because B MUST be dynamic
                if (FloatEquals(a->mass_inverse, 0.0f))
                {
                    PhysicsBody* temp = a;  // Store the static entity
                    a = b;                  // A is now the dynamic entity
                    b = temp;               // B is now the static entity
                }

                // Apply mtv accordingly now that we've made some guarantees
                // (This still isn't a generic algorithm, we're relying on the assumption that B is a plane)
                // (Later we will make this generic once you've completed your LE5 so we have MTVs)
                if (collision)
                {
                    a->vel = Vector2Reflect(a->vel, b->collider.normal);
                }
            }
        }
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

class FrictionScene : public Scene
{
public:
    void Load() final
    {
        plane = {};
        plane.pos = { 400.0f, 600.0f };
        plane.collider.type = COLLIDER_TYPE_PLANE;
        plane.collider.normal = Vector2Rotate(Vector2UnitX, -75.0F * DEG2RAD);

        Vector2 launch_force = Vector2UnitY * -10000.0f;

        //Circle 1 light sticky
        circle_light_sticky = {};
        circle_light_sticky.pos = { 100.0f, 400.0f };

        circle_light_sticky.mass_inverse = 1.0f / 2.0f;
        circle_light_sticky.coeff_friction = 0.1f;

        circle_light_sticky.gravity_scale = 1.0f;
        circle_light_sticky.drag = 1.0f;

        circle_light_sticky.collider.type = COLLIDER_TYPE_CIRCLE;
        circle_light_sticky.collider.radius = 20.0f;

        circle_light_sticky.force += launch_force;

        //Circle 2 light grippy
        circle_light_grippy = {};
        circle_light_grippy.pos = { 200.0f, 400.0f };

        circle_light_grippy.mass_inverse = 1.0f / 2.0f;
        circle_light_grippy.coeff_friction = 0.8f;

        circle_light_grippy.gravity_scale = 1.0f;
        circle_light_grippy.drag = 1.0f;

        circle_light_grippy.collider.type = COLLIDER_TYPE_CIRCLE;
        circle_light_grippy.collider.radius = 20.0f;

        circle_light_grippy.force += launch_force;

        //Circle 3 heavy sticky
        circle_heavy_sticky = {};
        circle_heavy_sticky.pos = { 300.0f, 400.0f };

        circle_heavy_sticky.mass_inverse = 1.0f / 8.0f;
        circle_heavy_sticky.coeff_friction = 0.1f;

        circle_heavy_sticky.gravity_scale = 1.0f;
        circle_heavy_sticky.drag = 1.0f;

        circle_heavy_sticky.collider.type = COLLIDER_TYPE_CIRCLE;
        circle_heavy_sticky.collider.radius = 20.0f;

        circle_heavy_sticky.force += launch_force;

        //Circle 4 heavy grippy
        circle_heavy_grippy = {};
        circle_heavy_grippy.pos = { 400.0f, 400.0f };

        circle_heavy_grippy.mass_inverse = 1.0f / 8.0f;
        circle_heavy_grippy.coeff_friction = 0.8f;

        circle_heavy_grippy.gravity_scale = 1.0f;
        circle_heavy_grippy.drag = 1.0f;

        circle_heavy_grippy.collider.type = COLLIDER_TYPE_CIRCLE;
        circle_heavy_grippy.collider.radius = 20.0f;

        circle_heavy_grippy.force += launch_force;
    }

    void Update(float dt) final
    {
        if (IsKeyDown(KEY_E))
            plane.collider.normal = Vector2Rotate(plane.collider.normal, 100.0f * DEG2RAD * dt);
        if (IsKeyDown(KEY_Q))
            plane.collider.normal = Vector2Rotate(plane.collider.normal, -100.0f * DEG2RAD * dt);

        //Circle 1 light sticky
        Vector2 mtv = Vector2Zeros;
        circle_light_sticky.collision = HitTestCirclePlane(circle_light_sticky.pos, circle_light_sticky.collider.radius, plane.pos, plane.collider.normal, &mtv);
        Vector2 fNet = Vector2Zeros;
        if (circle_light_sticky.collision)
        {
            fNet = CalculateFriction(circle_light_sticky, plane);
        }
        circle_light_sticky.pos += mtv;
        circle_light_sticky.force += fNet;

        //Circle 2 light grippy
        mtv = Vector2Zeros;
        circle_light_grippy.collision = HitTestCirclePlane(circle_light_grippy.pos, circle_light_grippy.collider.radius, plane.pos, plane.collider.normal, &mtv);
        fNet = Vector2Zeros;
        if (circle_light_grippy.collision)
        {
            fNet = CalculateFriction(circle_light_grippy, plane);
        }
        circle_light_grippy.pos += mtv;
        circle_light_grippy.force += fNet;

        //Circle 3 heavy sticky
        mtv = Vector2Zeros;
        circle_heavy_sticky.collision = HitTestCirclePlane(circle_heavy_sticky.pos, circle_heavy_sticky.collider.radius, plane.pos, plane.collider.normal, &mtv);
        fNet = Vector2Zeros;
        if (circle_heavy_sticky.collision)
        {
            fNet = CalculateFriction(circle_heavy_sticky, plane);
        }
        circle_heavy_sticky.pos += mtv;
        circle_heavy_sticky.force += fNet;

        //Circle 3 heavy sticky
        mtv = Vector2Zeros;
        circle_heavy_grippy.collision = HitTestCirclePlane(circle_heavy_grippy.pos, circle_heavy_grippy.collider.radius, plane.pos, plane.collider.normal, &mtv);
        fNet = Vector2Zeros;
        if (circle_heavy_grippy.collision)
        {
            fNet = CalculateFriction(circle_heavy_grippy, plane);
        }
        circle_heavy_grippy.pos += mtv;
        circle_heavy_grippy.force += fNet;

        UpdateMotion(&circle_light_sticky, dt);
        UpdateMotion(&circle_light_grippy, dt);
        UpdateMotion(&circle_heavy_sticky, dt);
        UpdateMotion(&circle_heavy_grippy, dt);
    }

    void Draw() final
    {
        DrawBody(plane, LIGHTGRAY);

        DrawBody(circle_light_sticky, RED);
        DrawBody(circle_light_grippy, GREEN);
        DrawBody(circle_heavy_sticky, BLUE);
        DrawBody(circle_heavy_grippy, YELLOW);

        DrawForces(circle_light_sticky, plane);
        DrawForces(circle_light_grippy, plane);
        DrawForces(circle_heavy_sticky, plane);
        DrawForces(circle_heavy_grippy, plane);
    }

private:

    // LE6 TODO -- setup the remaining 3 circles accordingly and add them to the simulation!
    PhysicsBody circle_light_sticky;
    PhysicsBody circle_light_grippy;
    PhysicsBody circle_heavy_sticky;
    PhysicsBody circle_heavy_grippy;
    PhysicsBody plane;

    // LE6 TODO -- complete this function
    void DrawForces(PhysicsBody circle, PhysicsBody plane)
    {
        Vector2 fg = GRAVITY / circle.mass_inverse;
        Vector2 fn = plane.collider.normal * Vector2Length(fg);
        Vector2 ff = Vector2Tangent(circle.vel * -circle.coeff_friction, plane.collider.normal);

        DrawLineEx(circle.pos, circle.pos + fn, 4.0f, GREEN);
        DrawLineEx(circle.pos, circle.pos + ff, 4.0f, ORANGE);
        DrawLineEx(circle.pos, circle.pos + fg, 4.0f, PURPLE);
        DrawLineEx(circle.pos, circle.pos + circle.vel * 0.5, 4.0f, RED);
    }

    Vector2 CalculateFriction(PhysicsBody circle, PhysicsBody plane)
    {
        Vector2 fg = GRAVITY / circle.mass_inverse;
        Vector2 fn = plane.collider.normal * Vector2Length(fg);
        Vector2 ff = Vector2Tangent(circle.vel * -circle.coeff_friction, plane.collider.normal);

        //We are already applying the force of gravity elsewhere so just add the remaining two factors in
        return fn + ff;
    }
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
    scene_names[SCENE_FRICTION] = "Friction Test";

    Scene* scenes[SCENE_COUNT];
    scenes[SCENE_CIRCLES] = new CircleTestScene;
    scenes[SCENE_CIRCLE_PLANE] = new CirclePlaneTestScene;
    scenes[SCENE_BALL_PIT] = new BallPitScene;
    scenes[SCENE_FRICTION] = new FrictionScene;

    int scene_index = SCENE_COUNT - 1;
    scenes[scene_index]->Load();
    SetWindowTitle(scene_names[scene_index]);

    int frame_count = 0;
    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_TAB))
        {
            scenes[scene_index]->Unload();
            ++scene_index %= SCENE_COUNT;
            scenes[scene_index]->Load();
            SetWindowTitle(scene_names[scene_index]);
        }

        if (IsKeyPressed(KEY_R))
        {
            scenes[scene_index]->Unload();
            scenes[scene_index]->Load();
        }

        // GetFrameTime() is unstable on frames 0 and 1
        if (frame_count >= 2)
        {
            float dt = GetFrameTime();
            scenes[scene_index]->Update(dt);
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawFPS(720, 10);
        scenes[scene_index]->Draw();
        EndDrawing();

        frame_count++;
    }

    scenes[scene_index]->Unload();
    for (int i = 0; i < SCENE_COUNT; i++)
        delete scenes[i];

    CloseAudioDevice();
    CloseWindow();
    return 0;
}