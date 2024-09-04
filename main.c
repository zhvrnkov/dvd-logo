#include "stdio.h"
#include "math.h"

#include "raylib.h"
#include "raymath.h"

int main()
{
    InitWindow(800, 600, "Playground");
    InitAudioDevice();

    Sound bounceSound = LoadSound("./assets/sound_jump-90516.wav");

    Texture2D texture = LoadTexture("./assets/dvd_logo.png");
    Rectangle sourceTextureRect;
    sourceTextureRect.x = 0;
    sourceTextureRect.y = 0;
    sourceTextureRect.width = texture.width;
    sourceTextureRect.height = texture.height;

    Rectangle destTextureRect;
    destTextureRect.x = 0;
    destTextureRect.y = 0;
    destTextureRect.width = 128;
    destTextureRect.height = 128;
    
    float mass = 1e1;
    Vector2 pos = {512, 512};
    Vector2 speed = {128, 0};
    float g = 9.8 * 256.0 / 10.0;
    Vector2 size = {destTextureRect.width / 2, destTextureRect.height / 2};
    const float k = 5e3;
    const float c = 5e0;
    
    SetTargetFPS(60);
    
    bool isDown = true;
    bool didBounceX = false;
    bool didBounceY = false;
    Vector2 lastWindowPosition = GetWindowPosition();
    lastWindowPosition.y = GetRenderHeight() - lastWindowPosition.y;
    Vector2 windowAcceleration = Vector2Zero();
    while (!WindowShouldClose()) {
        Vector2 windowPosition = GetWindowPosition();
        windowPosition.y = GetRenderHeight() - windowPosition.y;
        Vector2 deltaWindowPosition = Vector2Subtract(windowPosition, lastWindowPosition);
        lastWindowPosition = windowPosition;
        windowAcceleration = Vector2Add(Vector2Negate(Vector2Scale(deltaWindowPosition, 1e1)), windowAcceleration);
        windowAcceleration = Vector2Scale(windowAcceleration, 0.95);
        BeginDrawing();
        {
            float dt = 1.0 / 60.0;
            ClearBackground(GetColor(0));
            
            bool isBouncingX = false;
            bool isBouncingY = false;
            
            Vector2 imsize = size;
            Vector2 npos = pos;

            Vector2 force = {0, isDown ? -g * mass : g * mass};
            if (IsKeyDown(KEY_UP)) isDown = false;
            if (IsKeyDown(KEY_DOWN)) isDown = true;
            force.x += windowAcceleration.x;
            force.y += windowAcceleration.y;
            isBouncingX = didBounceX;
            isBouncingY = didBounceY;
            float y = fminf(npos.y - imsize.y, 0.0) + fmaxf(npos.y + imsize.y - GetScreenHeight(), 0.0);
            if (fabsf(y) > 0) {
                didBounceY = true;
                float s = speed.y;
                force.y = -k * y - c * s + force.y;
                imsize.y -= fabsf(y);
            } 

            float x = fminf(npos.x - imsize.x, 0.0) + fmaxf(npos.x + imsize.x - GetScreenWidth(), 0.0);
            if (fabsf(x) > 0) {
                didBounceX = true;
                float s = speed.x;
                force.x = -k * x - c * s + force.x;
                imsize.x -= fabsf(x);
            }
            if (fabsf(speed.y) < 0.1 && fabsf(speed.x) < 0.1) {
                goto draw;
            }
            Vector2 acceleration = {force.x / mass, force.y / mass};

            speed.x += acceleration.x * dt;
            speed.y += acceleration.y * dt;

            npos.x += speed.x * dt;
            npos.y += speed.y * dt;
            
            pos = npos;
            bool shouldPlaySoundX = didBounceX && !isBouncingX;
            bool shouldPlaySoundY = didBounceY && !isBouncingY;
            if (shouldPlaySoundX || shouldPlaySoundY) {
                didBounceX = false;
                didBounceY = false;
                float pitch = 0;
                float volume = 0;
                if (shouldPlaySoundX) {
                    pitch += fabsf(speed.x / 1e2f);
                    volume += fabsf(speed.x / 1e2f);
                }
                if (shouldPlaySoundY) {
                    pitch += fabsf(speed.y / 1e2f);
                    volume += fabsf(speed.y / 1e2f);
                }
                pitch = Clamp(pitch, 0.75, 2.0);
                volume = Clamp(volume, 0.1, 1.0);
            
                SetSoundPitch(bounceSound, pitch);
                SetSoundVolume(bounceSound, volume);

                if (IsSoundPlaying(bounceSound)) {
                    StopSound(bounceSound);
                }
                PlaySound(bounceSound);
            }
draw:
            {
            Vector2 origin = (Vector2) {pos.x, GetScreenHeight() - pos.y};
            Vector2 texOrigin = Vector2Subtract(origin, (Vector2) { imsize.x, imsize.y});
            Rectangle rect = destTextureRect;
            rect.x = texOrigin.x;
            rect.y = texOrigin.y;
            rect.width = imsize.x * 2.0;
            rect.height = imsize.y * 2.0;
            DrawEllipse(origin.x, origin.y, imsize.x, imsize.y, GetColor(0xFFFFFFFF));
            DrawTexturePro(texture, sourceTextureRect, rect, Vector2Zero(), 0.0, GetColor(0xFFFFFFFF));
            }
        }

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}