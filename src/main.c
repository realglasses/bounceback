#include <stdio.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

const int screenWidth = 1920;
const int screenHeight = 1080;
const int FPS = 60;

const float gravity = 980.0f;
const float size = 36.0f;
const float launchPower = 20.0f;
const float maxVelocity = 1500.0f;
const float maxMagnitude = 100.0f;
const float friction = 100.0f;

Texture2D hoopTexture;

typedef struct Triangle
{
    Vector2 a;
    Vector2 b;
    Vector2 c;
}Triangle;

typedef struct rectView
{
    Rectangle *items;
    size_t len;
}rectView;

typedef struct TriView
{
    Triangle *items;
    size_t len;
}TriView;

typedef struct Button
{
    bool active;
    int x;
    int y;
    int width;
    int height;
    int fontSize;
    Color rectColor;
    const char *text;
    Color textColor;
}Button;

typedef struct Hoop
{
    Vector2 a;
    Vector2 b;
}Hoop;

Rectangle rectangles[] = 
{
    {screenWidth/2, screenHeight - 100, screenWidth/2, 100},
    {screenWidth-500, screenHeight-1000, 500, 100}
};

Triangle triangles[] = 
{
    {{0, 0}, {0, 400}, {400, 0}},
    {{3*screenWidth/4, screenHeight-100}, {screenWidth, screenHeight-100}, {screenWidth, screenHeight-200}},
};

Button button1 = {0, screenWidth/2-100, screenHeight/2-50, 200, 100, 40, GRAY, "good job", RAYWHITE};

Hoop hoop1 = {{screenWidth/2-72+300, screenHeight/2}, {screenWidth/2+72+300, screenHeight/2}};

Vector2 closestPointOnSegment(Vector2 p, Vector2 a, Vector2 b)
{
    Vector2 ab = Vector2Subtract(b, a);
    float t = Vector2DotProduct(Vector2Subtract(p, a), ab) / Vector2DotProduct(ab, ab);

    t = fmaxf(0.0f, fminf(t, 1.0f));

    return Vector2Add(a, Vector2Scale(ab, t));
}

void collisionRect(Rectangle *rect, Vector2 *position, Vector2 *velocity)
{
    if (rect == NULL || position == NULL || velocity == NULL) return;

    if (!CheckCollisionCircleRec(*position, size, *rect)) return;

    float left = rect->x;
    float right = rect->x + rect->width;
    float top = rect->y;
    float bottom = rect->y + rect->height;

    float overlapLeft = position->x + size - left;
    float overlapRight = right - (position->x - size);
    float overlapTop = position->y + size - top;
    float overlapBottom = bottom - (position->y - size);

    float minOverlap = fminf(
        fminf(overlapLeft, overlapRight),
        fminf(overlapTop, overlapBottom)
    );

    if (minOverlap == overlapLeft)
    {
        position->x = left - size;
        velocity->x = -fabsf(velocity->x);
    }
    else if (minOverlap == overlapRight)
    {
        position->x = right + size;
        velocity->x = fabsf(velocity->x);
    }
    else if (minOverlap == overlapTop)
    {
        position->y = top - size;
        velocity->y = -fabsf(velocity->y);
    }
    else
    {
        position->y = bottom + size;
        velocity->y = fabsf(velocity->y);
    }
}

void collisionTri(Triangle *tri, Vector2 *position, Vector2 *velocity)
{
    if (tri == NULL || position == NULL || velocity == NULL) return;

    Vector2 closest[3] = {
        closestPointOnSegment(*position, tri->a, tri->b),
        closestPointOnSegment(*position, tri->b, tri->c),
        closestPointOnSegment(*position, tri->c, tri->a)
    };

    Vector2 normal = {0};
    float minDistance = INFINITY;

    for (int i = 0; i < 3; i++)
    {
        Vector2 difference = Vector2Subtract(*position, closest[i]);
        float distance = Vector2Length(difference);

        if (distance < minDistance)
        {
            minDistance = distance;
            normal = difference;
        }
    }

    if (minDistance >= size) return;

    if (minDistance > 0.0f)
        normal = Vector2Scale(normal, 1.0f / minDistance);
    else
        return;

    *position = Vector2Add(
        *position,
        Vector2Scale(normal, size - minDistance)
    );

    float dot = Vector2DotProduct(*velocity, normal);

    if (dot < 0.0f)
    {
        *velocity = Vector2Subtract(
            *velocity,
            Vector2Scale(normal, 2.0f * dot)
        );
    }
}

void collisionPoint(Vector2 point, Vector2 *position, Vector2 *velocity)
{
    float radius = 1.0f;
    float collisionDistance = size + radius;

    Vector2 difference = Vector2Subtract(*position, point);
    float distance = Vector2Length(difference);

    if (distance >= collisionDistance) return;

    Vector2 normal;

    if (distance > 0.0f)
        normal = Vector2Scale(difference, 1.0f / distance);
    else
        normal = (Vector2){0.0f, -1.0f};

    *position = Vector2Add(
        *position,
        Vector2Scale(normal, collisionDistance - distance)
    );

    float dot = Vector2DotProduct(*velocity, normal);

    if (dot < 0.0f)
    {
        *velocity = Vector2Subtract(
            *velocity,
            Vector2Scale(normal, 2.0f * dot)
        );
    }
}

void drawButton(Button button)
{
    DrawRectangle(
        button.x,
        button.y,
        button.width,
        button.height,
        button.rectColor
    );

    int textWidth = MeasureText(button.text, button.fontSize);

    int textX = button.x + (button.width - textWidth) / 2;
    int textY = button.y + (button.height - button.fontSize) / 2;

    DrawText(
        button.text,
        textX,
        textY,
        button.fontSize,
        button.textColor
    );
}

void drawHoop(Hoop hoop, Texture2D texture){
    DrawTexture(texture, hoop.a.x, hoop.a.y, WHITE);
}

bool passedThroughHoop(Hoop hoop, Vector2 previousPosition, Vector2 position, float radius) 
{
    float hoopY = hoop.a.y;

    bool crossedDown =
        previousPosition.y + radius < hoopY &&
        position.y + radius >= hoopY;

    bool withinOpening =
        position.x - radius > hoop.a.x &&
        position.x + radius < hoop.b.x;

    return crossedDown && withinOpening;
}

rectView level1Rects = {rectangles, sizeof(rectangles)/sizeof(rectangles[0])};
TriView level1Tri = {triangles, sizeof(triangles)/sizeof(triangles[0])};

int main(void)
{
    InitWindow(screenWidth, screenHeight, "title");
    SetTargetFPS(FPS);

    Vector2 position = {screenWidth / 2.0f, screenHeight / 2.0f};
    Vector2 velocity = {0.0f, 0.0f};

    hoopTexture = LoadTexture("assets/basketball net.png");
    bool isDragging = false;
    bool isPassedThroughHoop = false;
    while (!WindowShouldClose())
    {
        Vector2 prevPosition = position;
        const float dt = GetFrameTime();
        Vector2 mouse = GetMousePosition();

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

        for(int i = 0; i<level1Rects.len; i++)
        {
            collisionRect(&level1Rects.items[i], &position, &velocity);
        }

        for(int i = 0; i<level1Tri.len; i++){
            collisionTri(&level1Tri.items[i], &position, &velocity);
        }
        
        collisionPoint(hoop1.a, &position, &velocity);
        collisionPoint(hoop1.b, &position, &velocity);

        if (passedThroughHoop(hoop1, prevPosition, position, size)) button1.active = 1;

        BeginDrawing();
        ClearBackground(RAYWHITE);
        for(int i = 0; i<level1Rects.len; i++)
        {
            DrawRectangleRec(level1Rects.items[i], BLACK);
        }
        for(int i = 0; i<level1Tri.len; i++){
            DrawTriangle(level1Tri.items[i].a, level1Tri.items[i].b, level1Tri.items[i].c, BLACK);
        }

        drawHoop(hoop1, hoopTexture);

        if (button1.active) drawButton(button1);

        if (isDragging)
            DrawLineV(position, mouse, BLUE);

        DrawCircleV(position, size, RED);


        EndDrawing();
    }
    UnloadTexture(hoopTexture);
    CloseWindow();
    return 0;
}

