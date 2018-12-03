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

Sprite * loadSprite(refexport_t* re,
                    char * spriteID,
                    char * texturename,
                    int textureWidth, int textureHeight,
                    int xOffset, int yOffset,
                    int spriteWidth, int spriteHeight)
{
    Sprite * sprite;
    int x, y, n;
    unsigned char * textureData = 0;
    if (fileExists(texturename))
        textureData = stbi_load(texturename, &x, &y, &n, 4);
    else
    {
    }
    sprite = re->registerSprite(
        spriteID,
        texturename,
        textureData,
        textureWidth, textureHeight,
        xOffset, yOffset,
        spriteWidth, spriteHeight);
    return sprite;
}

void game_init(refexport_t* re)
{
    Entity checkerboard1;
    memcpy(checkerboard1.transform.modelMat, gModelMatrix, 16*sizeof(float));
    checkerboard1.entityType = SPRITE_E;
    checkerboard1.EntityDescriptor.sprite = loadSprite(re,
                                                       "spriteID_1",
                                                       "..\\assets\\uv_checkerboard.jpg",
                                                       1024, 1024,
                                                       0, 0,
                                                       124, 124);
    
    loadSprite(re,
               "spriteID_1",
               "..\\assets\\uv_checkerboard.jpg",
               1024, 1024,
               62*2, 62,
               248, 124);
    
    
    Entity azores;
    memcpy(azores.transform.modelMat, gModelMatrix, 16*sizeof(float));
    azores.entityType = SPRITE_E;
    azores.EntityDescriptor.sprite = loadSprite(re,
                                                "spriteID_2",
                                                "..\\assets\\azores.png",
                                                560, 144,
                                                0, 0,
                                                560, 144);
    Entity azores2;
    memcpy(azores2.transform.modelMat, gModelMatrix, 16*sizeof(float));
    azores2.entityType = SPRITE_E;
    azores2.EntityDescriptor.sprite = loadSprite(re,
                                                 "spriteID_3",
                                                 "..\\assets\\azores.png",
                                                 560, 144,
                                                 0, 0,
                                                 560, 144);
    
    azores.transform.xPos = 5;
    azores.transform.yPos = 5;
    azores.transform.xScale = 3.0f;
    azores.transform.yScale = 3.0f;
    checkerboard1.transform.xPos = 10;
    checkerboard1.transform.yPos = 9;
    checkerboard1.transform.xScale = 1.0f;
    checkerboard1.transform.yScale = 12.0f;
    addEntity(&azores);
    addEntity(&checkerboard1);
    
    float vertices[] = {
        -1, -1, 0,
        0, 1, 0,
        1, -1, 0
    };
    Mesh asteroidMesh = re->registerMesh(vertices, sizeof(vertices)/sizeof(vertices[0]));
    for (int i = 0;
         i < 100;
         i++)
    {
        Entity asteroid;
        asteroid.entityType = MESH_E;
        asteroid.EntityDescriptor.mesh = asteroidMesh;
        asteroid.transform.xPos = sin(i) * 10.0f;
        asteroid.transform.yPos = cos(i) * 2.0f;
        asteroid.transform.xScale = 1.0f;
        asteroid.transform.yScale = 1.0f;
        memcpy(asteroid.transform.modelMat, gModelMatrix, 16*sizeof(float));
        addEntity(&asteroid);
    }
    
    
    
    /*
    Entity juggler;
    unsigned char * jugglerSheet = stbi_load("..\\assets\\juggler\\spritesheet_juggler.png", &textureWidth, &textureHeight, &n, 4);
    int jugglerFrameCount = 10;
    for (int i =0;
    i < jugglerFrameCount;
    ++i)
    {
    re->registerSprite(
    &juggler.sprite,
    jugglerSheet,
    400, 65,
    i*40, 0,
    40,65);
    }
    */
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

void game_update_and_render(float dt, refexport_t* re)
{
    gRefdef.numSpriteEntities = gNumSpriteEntities;
    gRefdef.spriteEntities = gSpriteEntityList;
    gRefdef.numMeshEntities = gNumMeshEntities;
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