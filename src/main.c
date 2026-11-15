#include <stdio.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

const int screenWidth = 1920;
const int screenHeight = 1080;
const int FPS = 60;

const float gravity = 980.0f;
const float size = 36.0f;
const float launchPower = 10.0f;
const float maxVelocity = 1500.0f;
const float maxMagnitude = 100.0f;
const float friction = 100.0f;

typedef struct rectView{
    Rectangle* items;
    size_t len;
}rectView;

Rectangle rectangles[] = {
    {0, 0, 100, 100},
    {screenWidth/2, screenHeight - 100, screenWidth/2, 100},
};

rectView level1Rects = {rectangles, sizeof(rectangles)/sizeof(rectangles[0])};

int main(void)
{
    InitWindow(screenWidth, screenHeight, "title");
    SetTargetFPS(FPS);

    Vector2 position = {screenWidth / 2.0f, screenHeight / 2.0f};
    Vector2 velocity = {0.0f, 0.0f};

    bool isDragging = false;

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();
        Vector2 mouse = GetMousePosition();

        for(int i = 0; i<level1Rects.len; i++)
        {
            DrawRectangleRec(level1Rects.items[i], BLACK);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointCircle(mouse, position, size))
        {
            isDragging = true;
            velocity = (Vector2){0.0f, 0.0f};
        }

        if (isDragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            Vector2 vector = Vector2Subtract(position, mouse);

            if (Vector2Length(vector) > maxMagnitude)
                vector = Vector2Scale(Vector2Normalize(vector), maxMagnitude);

            velocity = Vector2Scale(
                vector,
                launchPower
            );

            float speed = Vector2Length(velocity);

            if (speed > maxVelocity)
                velocity = Vector2Scale(Vector2Normalize(velocity), maxVelocity);

            isDragging = false;
        }

        if (!isDragging)
        {

            velocity.y += gravity * dt;

            float speed = Vector2Length(velocity);

            if (speed > 0.0f)
            {
                float newSpeed = fmaxf(0.0f, speed - friction * dt);
                velocity = Vector2Scale(Vector2Normalize(velocity), newSpeed);
            }
            
            position.x += velocity.x * dt;
            position.y += velocity.y * dt;

            position.x = fmaxf(size, fminf(position.x, screenWidth - size));
            position.y = fmaxf(size, fminf(position.y, screenHeight - size));

            if (position.x - size <= 0 || position.x + size >= screenWidth)
            {
                velocity.x *= -1.0f;
            }

            if (position.y - size <= 0 || position.y + size >= screenHeight)
            {
                velocity.y *= -1.0f;
            }

        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (isDragging)
            DrawLineV(position, mouse, BLUE);

        DrawCircleV(position, size, RED);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}