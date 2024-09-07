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
    Texture2D texture;
    Rectangle sourceTextureRect;
} ObjectTextureDescriptor;

typedef struct {
    Color color;
} ObjectColorDescriptor;

typedef struct {
    ObjectDescriptor descriptor;
    ObjectSoundEffects sf;
    ObjectTextureDescriptor tex;
    ObjectColorDescriptor color;
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
        sf.sounds[i] = LoadSoundAlias(sound);
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

        SetSoundPitch(sound, pitch);
        SetSoundVolume(sound, volume);

        PlaySound(sound);
    }
}

typedef struct {
    Vector2 size;
    Vector2 pos;
} ObjectDrawDescriptor;

void DrawDescriptor(ObjectDrawDescriptor* descriptor, ObjectTextureDescriptor* tex, ObjectColorDescriptor* colDesc)
{
    Vector2 pos = descriptor->pos;
    Vector2 imsize = descriptor->size;
    Vector2 origin = (Vector2) { pos.x, GetScreenHeight() - pos.y };
    if (colDesc) {
        DrawEllipse(origin.x, origin.y, imsize.x, imsize.y, colDesc->color);
    }
    if (tex) {
        Vector2 texOrigin = Vector2Subtract(origin, (Vector2) { imsize.x, imsize.y });
        Rectangle rect;
        rect.x = texOrigin.x;
        rect.y = texOrigin.y;
        rect.width = imsize.x * 2.0;
        rect.height = imsize.y * 2.0;
        DrawTexturePro(tex->texture, tex->sourceTextureRect, rect, Vector2Zero(), 0.0, GetColor(0xFFFFFFFF));
    }
}

ObjectDrawDescriptor MakeObjectDrawDescriptor(Object* object, float dt, Vector2 extAcceleration, Vector2 extForce, float c, float u) 
{
    ObjectDrawDescriptor dd;
    Vector2 imsize = object->descriptor.size;
    float area = PI * imsize.x * imsize.y;
    Vector2 npos = object->descriptor.pos;
    float k = object->descriptor.stiffness;

    Vector2 force = Vector2Scale(extAcceleration, object->descriptor.mass);
    force = Vector2Add(force, extForce);
    Vector2 fritionForce = Vector2Zero();
    float y = fminf(npos.y - imsize.y, 0.0) + fmaxf(npos.y + imsize.y - GetScreenHeight(), 0.0);
    if (fabsf(y) > 0) {
        object->sf.didBounceY += 1;
        float s = object->descriptor.speed.y;
        float N = -k * y - c * s;
        force.y += N;
        imsize.y -= fabsf(y);
        imsize.x = area / (imsize.y * PI);
        
        fritionForce.x = -(object->descriptor.speed.x / fabsf(object->descriptor.speed.x)) * u * N;
    } else {
        object->sf.didBounceY = 0;
    }

    float x = fminf(npos.x - imsize.x, 0.0) + fmaxf(npos.x + imsize.x - GetScreenWidth(), 0.0);
    if (fabsf(x) > 0) {
        object->sf.didBounceX += 1;
        float s = object->descriptor.speed.x;
        float N = -k * x - c * s;
        force.x += N;
        imsize.x -= fabsf(x);
        imsize.y = area / (imsize.x * PI);

        fritionForce.y = (object->descriptor.speed.y / fabsf(object->descriptor.speed.y)) * u * N;
    } else {
        object->sf.didBounceX = 0;
    }
    
    force = Vector2Add(force, fritionForce);
    
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

    dd.pos = npos;
    dd.size = imsize;

    return dd;
}

Vector3 cosv3(Vector3 x)
{
    return (Vector3) { cosf(x.x), cosf(x.y), cosf(x.z) };
}

Color pallete(float t)
{
    Vector3 a = { 0.5, 0.5, 0.5 };
    Vector3 b = { 0.5, 0.5, 0.5 };
    Vector3 c = { 1.0, 1.0, 1.0 };
    Vector3 d = { 0.00, 0.10, 0.20 };
    
    Vector3 color = Vector3Add(a, Vector3Multiply(b, cosv3(Vector3Scale(Vector3Add(Vector3Scale(c, t), d), 6.28318f))));
    
    return ColorFromNormalized((Vector4) { color.x, color.y, color.z, 1.0 });
}

Vector2 gravity(const Object* from, const Object* to)
{
    const float G = 6.67 * 1e-4;
    Vector2 force = Vector2Zero();

    Vector2 center = (Vector2) { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    Vector2 radiusVector = Vector2Subtract(from->descriptor.pos, to->descriptor.pos);
    float radius = Vector2Length(radiusVector);
    float centrificForce = G * from->descriptor.mass * to->descriptor.mass / (radius * radius);
    radiusVector = Vector2Normalize(radiusVector);
    return Vector2Scale(radiusVector, centrificForce);
}

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(1200, 900, "Playground");
    InitAudioDevice();

    Sound bumpSound = LoadSound("./assets/sound_jump-90516.wav");

    Texture2D texture = LoadTexture("./assets/dvd_logo.png");
    Rectangle sourceTextureRect;
    sourceTextureRect.x = 0;
    sourceTextureRect.y = 0;
    sourceTextureRect.width = texture.width;
    sourceTextureRect.height = texture.height;

    Object object = {
        .descriptor = MakeObjectDescriptor(
            1e9,
            (Vector2) { GetScreenWidth() / 2.0 + 128, GetScreenHeight() / 2.0 },
            (Vector2) { 0, 32 },
            (Vector2) { 8, 8 },
            1e12),
        .sf = MakeObjectSoundEffects(bumpSound),
        .tex = {
            .sourceTextureRect = sourceTextureRect,
            .texture = texture
        }
    };
    
    Object object2 = {
        .descriptor = MakeObjectDescriptor(
            2e9,
            (Vector2) { GetScreenWidth() / 2.0 - 128, GetScreenHeight() / 2.0 },
            (Vector2) { 0, -32 },
            (Vector2) { 16, 16 },
            1e12),
        .sf = MakeObjectSoundEffects(bumpSound),
        .tex = {
            .sourceTextureRect = sourceTextureRect,
            .texture = texture
        }
    };

    float g = 9.8 * 256.0 / 10.0;
    const float c = 1e10; // energy lose coef
    const float u = 0.01; // friction coef
    int itersCount = 1000;

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
            float t = GetTime();
            float dt = 1.0 / 120.0;
            ClearBackground(GetColor(0));

            Vector2 extAcceleration = { 0, isDown ? -g : g };
            extAcceleration = Vector2Zero();
            extAcceleration = Vector2Add(extAcceleration, windowAcceleration);

            if (IsKeyDown(KEY_UP))
                isDown = false;
            if (IsKeyDown(KEY_DOWN))
                isDown = true;
            
            if (IsKeyDown(KEY_RIGHT))
                itersCount += 1;

            if (IsKeyDown(KEY_LEFT))
                itersCount += 1;
            
            if (itersCount < 100)
                itersCount = 100;
            
            ObjectDrawDescriptor drawDescriptors[2] = {0};
            
            for (int i = 0; i < itersCount / 100; i ++) {
                Vector2 obj1_2_obj2 = gravity(&object, &object2);
                Vector2 obj2_2_obj1 = gravity(&object2, &object);

                drawDescriptors[0] = MakeObjectDrawDescriptor(&object, dt, extAcceleration, obj2_2_obj1, c, u);
                object.color = (ObjectColorDescriptor) { pallete(0.6) };

                drawDescriptors[1] = MakeObjectDrawDescriptor(&object2, dt, extAcceleration, obj1_2_obj2, c, u);
                object2.color = (ObjectColorDescriptor) { pallete(0.1) };
            }

            PlaySoundEffect(&object);
            PlaySoundEffect(&object2);

            DrawDescriptor(&drawDescriptors[0], NULL, &object.color);
            DrawDescriptor(&drawDescriptors[1], NULL, &object2.color);
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
