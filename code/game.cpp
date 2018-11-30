#include "game.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//global_var Sprite sprite;
//global_var Sprite sprite2;
//global_var Spritesheet spriteSheet;
//global_var Shader shaders[MAX_SHADERS]; 

global_var Room gRoomList[MAX_SPRITES];
global_var int gNumRooms;
global_var Entity gEntitiesList[MAX_SPRITES];
global_var int gNumEntities;
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

void drawRoom(Room* room, refexport_t* re)
{
    // TODO(Michael): actually only needs metadata of the scene/room ???
    // eg get all the rendering related stuff out of the scene/room and
    // only send that to the renderer?
    //re->render(room);
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
    checkerboard1.sprite = loadSprite(re,
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
    azores.sprite = loadSprite(re,
                               "spriteID_2",
                               "..\\assets\\azores.png",
                               560, 144,
                               0, 0,
                               560, 144);
    Entity azores2;
    azores2.sprite = loadSprite(re,
                                "spriteID_3",
                                "..\\assets\\azores.png",
                                560, 144,
                                0, 0,
                                560, 144);
    
    addEntity(&azores);
    checkerboard1.xPos = 10;
    checkerboard1.yPos = 9;
    addEntity(&checkerboard1);
    
    
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

void addRoom(Room * room)
{
    if (gNumRooms >= MAX_SPRITES) return;
    gRoomList[gNumRooms] = *room;
    gNumRooms++;
}

void addEntity(Entity * entity)
{
    if (gNumEntities >= MAX_SPRITES) return;
    gEntitiesList[gNumEntities] = *entity;
    gNumEntities++;
}

void game_update_and_render(float dt, refexport_t* re)
{
    gRefdef.numEntities = gNumEntities;
    gRefdef.entities = gEntitiesList;
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