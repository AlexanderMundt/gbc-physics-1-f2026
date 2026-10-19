/*
Author:     Alexander Mundt 101632886
Class:      GAME2005
Professor:  Connor Smiley
Assignment: Assignment 1

---Task 2 Part C---
The two things I can think of that are causing the data discrepancy:
    1.  The simulation framerate. If our simulation were to run at 1000 fps instead of our 60
        the values would more closely resemble raw math calculations. This is due to how we check for collision
        and how with floating point numbers we can rarely ever pass float == conditions reliably. Overall, a 
        higher framerate will lead to more accuracy but will not solve the underlying issue.
    2.  We check for collision with the ground discreetly once per frame. We would need to have some sort of
        collision/physics interpolation going in order to capture the closet possible instant where the 
        simulation represents reality.
*/
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include <vector>
#include <cmath>
#include <cstdio>
#include <cassert>
#include <string>

//Constants
constexpr float GRAVITY = 20.0f;
constexpr float GROUND_Y = 700.0f;
constexpr Vector2 LAUNCH_POS = { 50.0f, GROUND_Y };
constexpr float LAUNCH_SPEED = 120.0f;

struct PhysicsBody
{
    Vector2 position;
    Vector2 velocity;
    Vector2 accel;
    float drag;
    float mass;

    Color color;

    bool grounded;

    std::vector<Vector2> pathToPlot;

    PhysicsBody()
    {
        position = Vector2Zeros;
        velocity = Vector2Zeros;
        accel = Vector2Zeros;
        drag = 1.0f;
        mass = 1.0f;

        color = BLACK;

        grounded = false;

        pathToPlot = std::vector<Vector2>();
        pathToPlot.push_back(position);
    }

    PhysicsBody(Vector2 pos, Vector2 vel)
    {
        position = pos;
        velocity = vel;
        accel = Vector2Zeros;
        drag = 1.0f;
        mass = 1.0f;

        color = BLACK;

        grounded = false;

        pathToPlot = std::vector<Vector2>();
        pathToPlot.push_back(position);
    }
};

struct Record
{
    float angle;
    float range;
    float time;

	Record()
	{
		angle = 0.0f;
		range = 0.0f;
		time = 0.0f;
	}

	Record(float ang)
	{
		angle = ang;
		range = CalculateRange();
		time = CalculateTime();
	}

	void SetAngle(float ang)
	{
		angle = ang;
	}

	float CalculateRange()
	{
		range = ((LAUNCH_SPEED * LAUNCH_SPEED) * sinf((2.0f * angle) * DEG2RAD)) / GRAVITY;
		return range;
	}

	float CalculateTime()
	{
		time = (2.0f * LAUNCH_SPEED * sinf(angle * DEG2RAD)) / GRAVITY;
		return time;
	}
};

//CSV save/load functions
static void SaveRecordsToCSV(const std::vector<Record>& records, const std::string& filename)
{
    float test_floats[] = { 1.1f, 2.2f, 3.3f, 4.4f, 5.5f };
    int test_ints[] = { 1, 2, 3, 4, 5 };

    char buffer[4096];
    int offset = 0;

	for (const Record& record : records)
	{
		offset += sprintf(buffer + offset, "%.4f,%.1f,%.4f\n", record.angle, record.range, record.time);
	}

    bool result = SaveFileText(filename.c_str(), buffer);
    assert(result);
}

int main()
{
    InitWindow(800, 800, "GAME-2005 Assignment1");
    InitAudioDevice();
    SetTargetFPS(60);

    Vector2 launchDir = Vector2Zeros;
    Vector2 launchVel = Vector2Zeros;

    int outputRecordIndex = 0;

    //Time variables
    float dt = 0.0f;
    float tt = 0.0f;

    //For the launched projectiles
    std::vector<PhysicsBody> bodies;

    //Input Records
    std::vector<Record> inputRecords
    {
        { 0.0f },
        { 3.1894f },
        { 6.4192f },
        { 9.7346f },
        { 13.1925f },
        { 16.8726f },
        { 20.9026f },
        { 25.5253f },
        { 31.3614f },
        { 45.0f },
    };

	//Write input records to a CSV
	SaveRecordsToCSV(inputRecords, "./InputRecords.csv");

    //Output Records
	std::vector<Record> outputRecords;
	for (const Record& iRec : inputRecords)
	{
		outputRecords.push_back(Record());
		outputRecords.back().SetAngle(iRec.angle);
	}

    //Make physics bodies
    for (Record& oRec : outputRecords)
    {
        launchDir = Vector2Rotate(Vector2UnitX, -oRec.angle * DEG2RAD);
        launchVel = launchDir * LAUNCH_SPEED;

        PhysicsBody body = PhysicsBody(LAUNCH_POS, launchVel);
        body.accel = { 0.0f, GRAVITY };
        body.color = GREEN;

        bodies.push_back(body);
    }

    while (!WindowShouldClose())
    {
        //This method of timing works better than using GetTime() because it starts accumulating when the simulation begins
        dt = GetFrameTime();  //Frame time - 16.66ms / 0.016s at 60 fps
        tt += dt;

        for (PhysicsBody& b : bodies)
        {
            b.velocity += Vector2Scale(b.accel, dt);
            b.velocity *= powf(b.drag, dt);
            b.position += Vector2Scale(b.velocity, dt);
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawFPS(720, 8);
        
        //Launch point circle and projection line
        DrawCircleV(LAUNCH_POS, 20.0f, RED);
        DrawLineEx(LAUNCH_POS, LAUNCH_POS + launchVel, 5.0f, ORANGE);
        
        //Draw the ground
		DrawLineEx(Vector2{ 0.0f, GROUND_Y }, Vector2{ 800.0f, GROUND_Y }, 2.0f, DARKGRAY);

        for (PhysicsBody& b : bodies)
        {
            if (!b.grounded)
            {
                //If the projectile has hit the ground stop it from moving
                if (b.position.y > GROUND_Y)
                {
                    b.color = GRAY;
                    b.accel = Vector2Zeros;
                    b.velocity = Vector2Zeros;
                    b.grounded = true;

                    //Record the final distance and flight time
                    outputRecords[outputRecordIndex].range = b.position.x - LAUNCH_POS.x;
                    outputRecords[outputRecordIndex].time = tt;

                    outputRecordIndex++;
                }
                //Only add the position to the path vector if the projectile is still moving
                else
                {
                    //Add that position to the vector of points that we will draw a line on
                    b.pathToPlot.push_back(b.position);
                }
            }

            //Draw the projectile at the new calculated position
            DrawCircleV(b.position, 15.0f, b.color);

            //Plot the path that the projectile has traveled 
            for (int i = 0, j = 1; j < b.pathToPlot.size(); i++, j++)
            {
                //From previous pos(i) to current pos(j)
                DrawLineEx(b.pathToPlot.at(i), b.pathToPlot.at(j), 1.5f, SKYBLUE);
            }
        }

        EndDrawing();
    }

    //Write output records to a CSV
    SaveRecordsToCSV(outputRecords, "./OutputRecords.csv");

    CloseAudioDevice();
    CloseWindow();
    return 0;
}