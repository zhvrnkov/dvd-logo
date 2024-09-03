#include "stdio.h"
#include "math.h"

#include "raylib.h"

typedef union {
    struct {
        float x;
        float y;
    };
    struct {
        float w;
        float h;
    };
} float2;

typedef union {
    struct {
        float2 pos;
        float2 size;
    };
} float4;

void CheckCollision(float* pos, float speed, float* size, float bound, float minSize, float maxSize, float dt, bool* didBounce) 
{
    if (*pos + *size >= bound || *pos - *size <= 0) {
        if (*size >= minSize) {
            *size -= 0.5;
        } else {
            if (*pos + *size >= bound) {
                *pos = bound - *size;
            } else {
                *pos = *size;
            }
            *pos += speed * dt;
            *didBounce = true;
        }
    } else {
        *size = *size >= maxSize ? maxSize : *size + 0.5;
    }
}

int main()
{
    
    // ds = v * dt
    // ds / dt = v

    // dv = a * dt
    // dv / dt = a
    
    // F = m * a
    // a = F / m => F is a vector

    InitWindow(1000, 800, "Playground");
    InitAudioDevice();
    
    float mass = 1.0;
    float2 pos = {256, 256};
    float2 speed = {256, 0};
    float2 acceleration = {0, -9.8 * 256.0 / 10.0};
    float2 r = {32, 32};
    float2 min_r = {r.x * 0.8, r.y * 0.8};
    float2 max_r = r;
    float wallHardness = 0.95;

    Sound bounceSound = LoadSound("./assets/sound_jump-90516.wav");
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            float dt = GetFrameTime();
            ClearBackground(GetColor(0));
            
            bool didBounceX = false;
            bool didBounceY = false;
            
            float2 force = { mass * acceleration.x, mass * acceleration.y };

            float2 npos = pos;

            npos.x += speed.x * dt;
            npos.y += speed.y * dt;
            
            CheckCollision(&npos.x, speed.x, &r.x, GetScreenWidth(), min_r.x, max_r.x, dt, &didBounceX);
            CheckCollision(&npos.y, speed.y, &r.y, GetScreenHeight(), min_r.y, max_r.y, dt, &didBounceY);
            pos = npos;
            if (didBounceX) {
                speed.x *= -wallHardness;
            }
            if (didBounceY) {
                speed.y *= -wallHardness;
            }

            speed.x += acceleration.x * dt;
            speed.y += acceleration.y * dt;

            bool didBounce = didBounceX || didBounceY;
            if (didBounce) {
                printf("%f %f %f\n", speed.x, speed.y, wallHardness);
                if (IsSoundPlaying(bounceSound)) {
                    StopSound(bounceSound);
                }
                PlaySound(bounceSound);
            }

            DrawEllipse(pos.x, GetScreenHeight() - pos.y, r.w, r.h, GetColor(0xFF0000FF));
        }

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}