#include "math.h"
#include "stdio.h"

#include "raylib.h"
#include "raymath.h"

typedef struct {
    float mass;
    Vector2 pos;
    Vector2 speed;
    Vector2 size;
    float stiffness;
} ObjectDescriptor;

typedef struct {
    Sound sounds[4];
    int soundIndex;
    int didBounceX;
    int didBounceY;
} ObjectSoundEffects;

typedef struct {
    ObjectDescriptor descriptor;
    ObjectSoundEffects sf;
} Object;

ObjectDescriptor MakeObjectDescriptor(float mass, Vector2 pos, Vector2 speed, Vector2 size, float stiffness)
{
    ObjectDescriptor object = {0};
    object.mass = mass;
    object.pos = pos;
    object.speed = speed;
    object.size = size;
    object.stiffness = stiffness;
    return object;
}

ObjectSoundEffects MakeObjectSoundEffects(Sound sound)
{
    ObjectSoundEffects sf;
    for (int i = 0; i < 4; i++) {
        sf.sounds[0] = LoadSoundAlias(sound);
    }
    sf.soundIndex = 0;
    sf.didBounceX = 0;
    sf.didBounceY = 0;
    return sf;
}

void PlaySoundEffect(Object* object)
{
    ObjectSoundEffects* sf = &object->sf;
    bool shouldPlaySoundX = sf->didBounceX == 1;
    bool shouldPlaySoundY = sf->didBounceY == 1;
    if (shouldPlaySoundX || shouldPlaySoundY) {
        Sound sound = sf->sounds[sf->soundIndex % 4];
        sf->soundIndex += 1;
        float pitch = 0;
        float volume = 0;
        if (shouldPlaySoundX) {
            pitch += fabsf(object->descriptor.speed.x / 1e2f);
            volume += fabsf(object->descriptor.speed.x / 1e2f);
        }
        if (shouldPlaySoundY) {
            pitch += fabsf(object->descriptor.speed.y / 1e2f);
            volume += fabsf(object->descriptor.speed.y / 1e2f);
        }
        pitch = Clamp(pitch, 0.75, 2.0);
        volume = Clamp(volume, 0.1, 1.0);
        printf("play %f %f\n", pitch, volume);

        SetSoundPitch(sound, pitch);
        SetSoundVolume(sound, volume);

        PlaySound(sound);
    }
}

typedef struct {
    Vector2 size;
    Vector2 pos;
} ObjectDrawDescriptor;

ObjectDrawDescriptor MakeObjectDrawDescriptor(Object* object, float dt, Vector2 extAcceleration, float c) 
{
    ObjectDrawDescriptor dd;
    Vector2 imsize = object->descriptor.size;
    Vector2 npos = object->descriptor.pos;
    float k = object->descriptor.stiffness;

    Vector2 force = Vector2Scale(extAcceleration, object->descriptor.mass);
    float y = fminf(npos.y - imsize.y, 0.0) + fmaxf(npos.y + imsize.y - GetScreenHeight(), 0.0);
    if (fabsf(y) > 0) {
        object->sf.didBounceY += 1;
        float s = object->descriptor.speed.y;
        force.y += -k * y - c * s;
        imsize.y -= fabsf(y);
    } else {
        object->sf.didBounceY = 0;
    }

    float x = fminf(npos.x - imsize.x, 0.0) + fmaxf(npos.x + imsize.x - GetScreenWidth(), 0.0);
    if (fabsf(x) > 0) {
        object->sf.didBounceX += 1;
        float s = object->descriptor.speed.x;
        force.x += -k * x - c * s;
        imsize.x -= fabsf(x);
    } else {
        object->sf.didBounceX = 0;
    }
    
    dd.pos = npos;
    dd.size = imsize;

    if (fabsf(object->descriptor.speed.y) < 0.1 && fabsf(object->descriptor.speed.x) < 0.1 && fabsf(force.y) < 0.1 && fabsf(force.x) < 0.1) {
        return dd;
    }

    Vector2 acceleration = { force.x / object->descriptor.mass, force.y / object->descriptor.mass };

    object->descriptor.speed.x += acceleration.x * dt;
    object->descriptor.speed.y += acceleration.y * dt;

    npos.x += object->descriptor.speed.x * dt;
    npos.y += object->descriptor.speed.y * dt;

    object->descriptor.pos = npos;

    return dd;
}

int main()
{
    InitWindow(800, 600, "Playground");
    InitAudioDevice();

    Sound bumpSound = LoadSound("./assets/sound_jump-90516.wav");

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

    Object object = {
        .descriptor = MakeObjectDescriptor(
            1e1,
            (Vector2) { 512, 512 },
            (Vector2) { 128, 0 },
            (Vector2) { destTextureRect.width / 2, destTextureRect.height / 2 },
            1e3),
        .sf = MakeObjectSoundEffects(bumpSound)
    };


    float g = 9.8 * 256.0 / 10.0;
    const float c = 5e0;

    SetTargetFPS(60);

    bool isDown = true;
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

            Vector2 extAcceleration = { 0, isDown ? -g : g };
            extAcceleration = Vector2Add(extAcceleration, windowAcceleration);
            if (IsKeyDown(KEY_UP))
                isDown = false;
            if (IsKeyDown(KEY_DOWN))
                isDown = true;

            ObjectDrawDescriptor drawDescriptor = MakeObjectDrawDescriptor(&object, dt, extAcceleration, c);
            PlaySoundEffect(&object);
draw: 
        {
            Vector2 pos = drawDescriptor.pos;
            Vector2 imsize = drawDescriptor.size;
            Vector2 origin = (Vector2) { pos.x, GetScreenHeight() - pos.y };
            Vector2 texOrigin = Vector2Subtract(origin, (Vector2) { imsize.x, imsize.y });
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

// TODO: bouncing in circle
// TODO: more objects and collisions between objects
// TODO: create sound from vibrations and etc (simulate sound, not just play some wav)
