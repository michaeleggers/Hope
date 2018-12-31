#include "game.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

global_var int gNumRooms;
global_var Entity gSpriteEntityList[MAX_SPRITES];
global_var Entity gMeshEntityList[MAX_MESHES];
global_var int gNumSpriteEntities;
global_var int gNumMeshEntities;
global_var Refdef gRefdef;

Background loadBackground(char * file)
{
    Background bg;
    // check if file exists!
    if (!fileExists(file))
    {
        bg.imageFile = 0;
        printf("fuck! Failed to load %s!\n", file);
    }
    else
    {
        bg.imageFile = file;
    }
    return bg;
}

Object loadObject(char * file)
{
    Object obj;
    // check if file exists!
    if (!fileExists(file))
    {
        obj.imageFile = 0;
        printf("fuck! Failed to load %s!\n", file);
    }
    else
    {
        obj.imageFile = file;
    }
    return obj;
}

void game_init(refexport_t* re)
{
    Entity azores;
    memcpy(azores.transform.modelMat, gModelMatrix, 16*sizeof(float));
    azores.entityType = SPRITE_E;
    int x, y, n;
    unsigned char * azoresImageData = 0;
    if (fileExists("..\\assets\\azores.png"))
        azoresImageData = stbi_load("..\\assets\\azores.png", &x, &y, &n, 4);
    Sprite azoresSprite = re->registerSprite("..\\assets\\azores.png",
                                             azoresImageData,
                                             560, 144,
                                             0, 0,
                                             560, 144);
    azores.transform.xPos = 5;
    azores.transform.yPos = 5;
    azores.transform.xScale = 3.0f;
    azores.transform.yScale = 3.0f;
    azores.sprite = azoresSprite;
    
    
    Entity azores2;
    memcpy(azores2.transform.modelMat, gModelMatrix, 16*sizeof(float));
    azores2.entityType = SPRITE_E;
    Sprite azoresSprite2 = re->registerSprite("..\\assets\\azores.png",
                                              azoresImageData,
                                              560, 144,
                                              0, 0,
                                              560, 144);
    azores2.transform.xPos = -10;
    azores2.transform.yPos = 0;
    azores2.transform.xScale = 7.0f;
    azores2.transform.yScale = 7.0f;
    azores2.sprite = azoresSprite2;
    re->addSpriteFrame(&azores2.sprite, 50, 50, 50, 50);
    re->addSpriteFrame(&azores2.sprite, 300, 0, 10, 10);
    azores2.sprite.currentFrame = 2;
    
    addEntity(&azores);
    addEntity(&azores2);
    
    float vertices[] = {
        -1, -1, 0,
        0, 1, 0,
        1, -1, 0
    };
    Mesh asteroidMesh = re->registerMesh(vertices, sizeof(vertices)/sizeof(vertices[0]));
    for (int i = 0;
         i < 10;
         i++)
    {
        Entity asteroid;
        asteroid.entityType = MESH_E;
        asteroid.mesh = asteroidMesh;
        asteroid.transform.xPos = 0;
        asteroid.transform.yPos = 0;
        asteroid.transform.xScale = 1.0f;
        asteroid.transform.yScale = 1.0f;
        memcpy(asteroid.transform.modelMat, gModelMatrix, 16*sizeof(float));
        addEntity(&asteroid);
    }
}

void addEntity(Entity * entity)
{
    switch (entity->entityType)
    {
        case SPRITE_E:
        {
            if (gNumSpriteEntities >= MAX_SPRITES) return;
            gSpriteEntityList[gNumSpriteEntities] = *entity;
            gNumSpriteEntities++;
        }
        break;
        
        case MESH_E:
        {
            if (gNumMeshEntities >= MAX_MESHES) return;
            gMeshEntityList[gNumMeshEntities] = *entity;
            gNumMeshEntities++;
        }
        break;
    }
}

bool keyPressed(Keyboard* keyboard, Keycode keycode)
{
    if (keyboard->keycodes[keycode] && !keyboard->prevKeycodes[keycode])
    {
        keyboard->prevKeycodes[keycode] = 1;
        return true;
    }
    else if (!keyboard->keycodes[keycode] && keyboard->prevKeycodes[keycode])
    {
        keyboard->prevKeycodes[keycode] = 0;
        return false;
    }
    return false;
}

bool keyDown(Keyboard* keyboard, Keycode keycode)
{
    if (keyboard->keycodes[keycode])
        return true;
    return false;
}

bool keyUp(Keyboard* keyboard, Keycode keycode)
{
    if (!keyboard->keycodes[keycode] && keyboard->prevKeycodes[keycode])
    {
        keyboard->prevKeycodes[keycode] = 0;
        return true;
    }
    else if (keyboard->keycodes[keycode] && !keyboard->prevKeycodes[keycode])
    {
        keyboard->prevKeycodes[keycode] = 1;
        return false;
    }
    return false;
}

float p = 0.0f;
float velocity = 0.003f;
void game_update_and_render(float dt, Controller* controller, Keyboard* keyboard, refexport_t* re)
{
    static float posX = 0.0f;
    static float scaleY = 1.0f;
    
    Entity * spriteEntity = gSpriteEntityList;
    for (int i = 0;
         i < gNumSpriteEntities;
         ++i)
    {
        //spriteEntity->transform.xPos = 10.0f*sin(posX); //dt/1000.0f * velocity + spriteEntity->transform.xPos;
        
        if (controller->dpadUp || keyPressed(keyboard, ARROW_UP))
        {
            printf("DPAD UP pressed\n");
            spriteEntity->transform.yPos += 0.07f * dt/1000;
        }
        
        if (controller->dpadDown || keyDown(keyboard, ARROW_DOWN))
        {
            printf("DPAD DOWN pressed\n");
            spriteEntity->transform.yPos -= 0.07f * dt/1000;
        }
        
        if (controller->dpadLeft || keyUp(keyboard, ARROW_LEFT))
        {
            printf("DPAD LEFT pressed\n");
            spriteEntity->transform.xPos -= 0.07f * dt/1000;
        }
        
        if (controller->dpadRight || keyboard->keycodes[ARROW_RIGHT])
        {
            printf("DPAD RIGHT pressed\n");
            spriteEntity->transform.xPos += 0.07f * dt/1000;
        }
        if (controller->dpadA)
        {
            spriteEntity->transform.xScale += 0.01f * dt/1000;
            spriteEntity->transform.yScale += 0.01f * dt/1000;
        }
        if (controller->dpadX)
        {
            spriteEntity->transform.xScale -= 0.01f * dt/1000;
            spriteEntity->transform.yScale -= 0.01f * dt/1000;
        }
        
        spriteEntity++;
    }
    posX += dt * 0.00001f;
    scaleY += dt * 0.000001f;
    
    gRefdef.numSpriteEntities = gNumSpriteEntities;
    gRefdef.spriteEntities = gSpriteEntityList;
    // gRefdef.numMeshEntities = gNumMeshEntities;
    gRefdef.meshEntities = gMeshEntityList;
    re->renderFrame(&gRefdef);
}

// NOTE(Michael): Not in use yet. Maybe reverse the control so that
// the game loop is controlled from the game and not the platform layer.
void game_loop()
{
    bool running = false;
    
    while (running)
    {
        update_messages();
    }
}