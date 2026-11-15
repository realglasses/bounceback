#include <stdio.h>
#include <math.h>
#include <raylib.h>

const int screenWidth = 800;
const int screenHeight = 800;
const int FPS = 60;

typedef struct ScreenPoint
{
    int x;
    int y;
}ScreenPoint;

typedef struct FloatPoint{
    float x;
    float y;
}FloatPoint;

ScreenPoint normalize(FloatPoint p)
{
    ScreenPoint q = {(int) ((p.x+1)/2*screenWidth), (int) ((1-p.y)/2*screenHeight)};
    return q;
}

FloatPoint project(float x, float y, float z)
{
    FloatPoint p = {x/z, y/z};
    return p;
}

FloatPoint point(float x, float y)
{
    FloatPoint p = {x, y};
    return p;
}

int main(void)
{

    InitWindow(screenWidth, screenHeight, "title");
    SetTargetFPS(FPS);

    float dz = 0.0f;

    while(!WindowShouldClose())
    {
        const float dt = 1.0f / FPS;
        dz += 1*dt;
        ScreenPoint p = normalize(project(0.5, 0.5, 1 + dz));
        ScreenPoint q = normalize(project(-0.5, 0.5, 1 + dz));
        ScreenPoint r = normalize(project(0.5, -0.5, 1 + dz));
        ScreenPoint s = normalize(project(-0.5, -0.5, 1 + dz));
        const int size = 20;


        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangle(p.x-size/2, p.y-size/2, size, size, RED);
        DrawRectangle(q.x-size/2, q.y-size/2, size, size, RED);
        DrawRectangle(r.x-size/2, r.y-size/2, size, size, RED);
        DrawRectangle(s.x-size/2, s.y-size/2, size, size, RED);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}