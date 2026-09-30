#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include <cstdio>
#include <cassert>
#include <iostream>

void Save()
{
    float test_floats[] = { 1.1f, 2.2f, 3.3f, 4.4f, 5.5f };
    int test_ints[] = { 1, 2, 3, 4, 5 };

    char buffer[4096];
    int offset = 0;

    //The return pushes the offset forward an amount equal to the number of chars written
    offset += sprintf(buffer + offset, "%i,%i,%i,%i,%i\n", test_ints[0], test_ints[1], test_ints[2], test_ints[3], test_ints[4]);
    offset += sprintf(buffer + offset, "%.1f,%.1f,%.1f,%.1f,%.1f\n", test_floats[0], test_floats[1], test_floats[2], test_floats[3], test_floats[4]);
    bool result = SaveFileText("./Example.csv", buffer);
    assert(result);
}

void Load()
{
    float test_ints[5];
    float test_floats[5];

    char* buffer = LoadFileText("./Example.csv");

    int offset = 0;
    offset += 2 * sscanf(buffer + offset, "%i,%i,%i,%i,%i\n", &test_ints[0], &test_ints[1], &test_ints[2], &test_ints[3], &test_ints[4]);
    offset += 2 * sscanf(buffer + offset, "%.1f,%.1f,%.1f,%.1f,%.1f\n", &test_floats[0], &test_floats[1], &test_floats[2], &test_floats[3], &test_floats[4]);
    
}

int main()
{
    InitWindow(800, 800, "Physics-1");
    InitAudioDevice();
    SetTargetFPS(60);

    Save();
    Load();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);

        DrawCircleV(GetMousePosition(), 20.0f, RED);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
