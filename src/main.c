#include <stdio.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

enum { screenWidth = 1920, screenHeight = 1080 };

const int FPS = 60;

const float gravity = 980.0f;
const float size = 36.0f;
const float launchPower = 20.0f;
const float maxVelocity = 1500.0f;
const float maxMagnitude = 100.0f;
const float friction = 100.0f;

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define HOOP_WIDTH 144
#define HOOP(x, y) {{(x), (y)}, {(x) + HOOP_WIDTH, (y)}}

Texture2D hoopTexture;
Texture2D ballTexture;

typedef struct Triangle
{
    Vector2 a;
    Vector2 b;
    Vector2 c;
}Triangle;

typedef struct RectView
{
    Rectangle *items;
    size_t len;
}RectView;

typedef struct TriView
{
    Triangle *items;
    size_t len;
}TriView;

typedef struct Text
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
}Text;

typedef struct Hoop
{
    Vector2 a;
    Vector2 b;
}Hoop;

typedef struct Level
{
    RectView rectangles;
    TriView triangles;
    Hoop hoop;
    Vector2 start;
}Level;

// tutorial ig
static Rectangle level1Rects[] =
{
    {screenWidth/2, screenHeight - 100, screenWidth/2, 100},
    {screenWidth-500, screenHeight-1000, 500, 100}
};

static Triangle level1Tris[] =
{
    {{0, 0}, {0, 400}, {400, 0}},
    {{3*screenWidth/4, screenHeight-100}, {screenWidth, screenHeight-100}, {screenWidth, screenHeight-200}},
};

// lvl 2
static Rectangle level2Rects[] =
{
    {900, 600, 120, 480},
    {1644, 250, 30, 260},
};

static Triangle level2Tris[] =
{
    {{700, 0}, {1100, 0}, {900, 250}},
    {{1250, screenHeight}, {1450, screenHeight}, {1350, 950}},
};

// lvl 3
static Rectangle level3Rects[] =
{
    {0, 450, 700, 60},
    {1100, 0, 80, 300},
    {1100, 500, 80, 580},
};

static Triangle level3Tris[] =
{
    {{screenWidth, 860}, {screenWidth, screenHeight}, {1700, screenHeight}}, 
};

// lvl 4
static Rectangle level4Rects[] =
{
    {0, 250, 700, 50}, 
    {935, 0, 50, 900},
    {150, 500, 785, 50},
    {0, 750, 700, 50},
    {150, 900, 835, 50},
    {1100, 180, 50, 900},
};

static Triangle level4Tris[] = 
{
    {{1200, 0}, {screenWidth, screenHeight - 300}, {screenWidth, 0}},
    {{1150, 180}, {screenWidth-100, screenHeight}, {1150, screenHeight}},
};

//lvl 5
static Rectangle level5Rects[] =
{
    {800, 450, 60, 630},
    {1100, 600, 60, 480},
};

static Triangle level5Tris[] =
{
    {{860, 700}, {1000, 770}, {860, 770}},
};

static Level levels[] =
{
    {
        {level1Rects, ARRAY_LEN(level1Rects)},
        {level1Tris,  ARRAY_LEN(level1Tris)},
        HOOP(1188, 540),
        {screenWidth/2.0f, screenHeight/2.0f}
    },
    {
        {level2Rects, ARRAY_LEN(level2Rects)},
        {level2Tris,  ARRAY_LEN(level2Tris)},
        HOOP(1500, 400),
        {150.0f, screenHeight - 36.0f}
    },
    {
        {level3Rects, ARRAY_LEN(level3Rects)},
        {level3Tris,  ARRAY_LEN(level3Tris)},
        HOOP(1400, 850),
        {150.0f, 410.0f}
    },
    {
        {level4Rects, ARRAY_LEN(level4Rects)},
        {level4Tris, ARRAY_LEN(level4Tris)},
        HOOP(1763, 900),
        {150.0f, 220.0f}
    },
    {
        {level5Rects, ARRAY_LEN(level5Rects)},
        {level5Tris,  ARRAY_LEN(level5Tris)},
        HOOP(908, 900),
        {150.0f, screenHeight - 36.0f}
    },
};



#define LEVEL_COUNT ARRAY_LEN(levels)

Text nextButton = {false, screenWidth/2-100, screenHeight/2-50, 200, 100, 40, GRAY, "next level", RAYWHITE};

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

bool passedThroughHoop(Hoop hoop, Vector2 previousPosition, Vector2 position)
{
    float hoopY = hoop.a.y;

    if (!(previousPosition.y < hoopY && position.y >= hoopY)) return false;

    float t = (hoopY - previousPosition.y) / (position.y - previousPosition.y);
    float x = previousPosition.x + (position.x - previousPosition.x) * t;

    return x > hoop.a.x && x < hoop.b.x;
}

bool textClicked(Text text, Vector2 mouse)
{
    if (!text.active) return false;

    Rectangle bounds = {text.x, text.y, text.width, text.height};

    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
           CheckCollisionPointRec(mouse, bounds);
}

void drawText(Text text)
{
    DrawRectangle(text.x, text.y, text.width, text.height, text.rectColor);

    int textWidth = MeasureText(text.text, text.fontSize);

    int textX = text.x + (text.width - textWidth) / 2;
    int textY = text.y + (text.height - text.fontSize) / 2;

    DrawText(text.text, textX, textY, text.fontSize, text.textColor);
}

void drawHoop(Hoop hoop, Texture2D texture)
{
    DrawTexture(texture, hoop.a.x, hoop.a.y, WHITE);
}

void drawTriangle(Triangle t, Color color)
{
    float cross = (t.b.x - t.a.x) * (t.c.y - t.a.y)
                - (t.b.y - t.a.y) * (t.c.x - t.a.x);

    if (cross > 0.0f)
        DrawTriangle(t.a, t.c, t.b, color);
    else
        DrawTriangle(t.a, t.b, t.c, color);
}

void drawLevel(const Level *level)
{
    for (size_t i = 0; i < level->rectangles.len; i++)
        DrawRectangleRec(level->rectangles.items[i], BLACK);

    for (size_t i = 0; i < level->triangles.len; i++)
        drawTriangle(level->triangles.items[i], BLACK);

    drawHoop(level->hoop, hoopTexture);
}

void updateLevelCollisions(const Level *level, Vector2 *position, Vector2 *velocity)
{
    for (size_t i = 0; i < level->rectangles.len; i++)
        collisionRect(&level->rectangles.items[i], position, velocity);

    for (size_t i = 0; i < level->triangles.len; i++)
        collisionTri(&level->triangles.items[i], position, velocity);

    collisionPoint(level->hoop.a, position, velocity);
    collisionPoint(level->hoop.b, position, velocity);
}

int main(void)
{
    InitWindow(screenWidth, screenHeight, "basketball");
    SetTargetFPS(FPS);

    size_t currentLevel = 0;

    Vector2 position = levels[currentLevel].start;
    Vector2 velocity = {0.0f, 0.0f};

    hoopTexture = LoadTexture("assets/basketball net.png");
    ballTexture = LoadTexture("assets/basketball.png");
    bool isDragging = false;

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();
        Vector2 mouse = GetMousePosition();

        if (textClicked(nextButton, mouse))
        {
            currentLevel = (currentLevel + 1) % LEVEL_COUNT;
            position = levels[currentLevel].start;
            velocity = (Vector2){0.0f, 0.0f};
            isDragging = false;
            nextButton.active = false;
        }

        const Level *level = &levels[currentLevel];
        Vector2 prevPosition = position;

        if (!nextButton.active &&
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointCircle(mouse, position, size))
        {
            isDragging = true;
            velocity = (Vector2){0.0f, 0.0f};
        }

        if (isDragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            Vector2 vector = Vector2Subtract(position, mouse);

            if (Vector2Length(vector) > maxMagnitude)
                vector = Vector2Scale(Vector2Normalize(vector), maxMagnitude);

            velocity = Vector2Scale(vector, launchPower);

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

        updateLevelCollisions(level, &position, &velocity);

        if (!nextButton.active && passedThroughHoop(level->hoop, prevPosition, position))
        {
            nextButton.text = (currentLevel + 1 < LEVEL_COUNT) ? "next level" : "play again";
            nextButton.active = true;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        drawLevel(level);

        DrawText(TextFormat("level %d / %d", (int)currentLevel + 1, (int)LEVEL_COUNT),
                 450, 20, 30, DARKGRAY);

        if (nextButton.active) drawText(nextButton);

        if (isDragging)
            DrawLineV(position, mouse, BLUE);

        DrawTexture(ballTexture, position.x-size, position.y-size, WHITE);

        EndDrawing();
    }

    UnloadTexture(hoopTexture);
    UnloadTexture(ballTexture);
    CloseWindow();
    return 0;
}