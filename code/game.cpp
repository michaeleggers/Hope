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

void game_init(refexport_t* re)
{
    
    // load "room"
    
    // TODO(Michael): the order of registration matters at the moment,
    // because renderer will draw paint over last pixels...
    // TODO(Michael): later the image file will not define the sprites
    // size. It should be determined by user defined values from asset file.
    // Also, the filename will come from asset file.
    Entity testRoom3;
    int textureWidth, textureHeight, n;
    unsigned char * testRoom3image = 0;
    if (fileExists("..\\assets\\uv_checkerboard.jpg"))
    {
        testRoom3image = stbi_load("..\\assets\\uv_checkerboard.jpg", &textureWidth, &textureHeight, &n, 4);
    }
    else
    {
        // TODO(Michael): load random values into Sprite resource data for debug
        textureWidth = 560; // from asset file
        textureHeight= 144; // from asset file
        /*
        testRoom3image = (unsigned char*)malloc(sizeof(unsigned char) * width * height);
        for (int row = 0;
             row < height;
             row++)
        {
            for (int col = 0;
                 col < width;
                 col++)
            {
                *testRoom3image = 0x12;
                testRoom3image++;
            }
        }
        */
    }
    int spriteWidth = 248;
    int spriteHeight = 124;
    int xOffset = 62;
    int yOffset = 62;
    testRoom3.sprite = re->registerSprite(
        "..\\assets\\uv_checkerboard.jpg", // string just for texture database
        testRoom3image,
        textureWidth, textureHeight,
        xOffset, yOffset,
        spriteWidth, spriteHeight);
    testRoom3.sprite = re->registerSprite(
        "..\\assets\\uv_checkerboard.jpg", // string just for texture database
        testRoom3image,
        textureWidth, textureHeight,
        xOffset*2, yOffset,
        spriteWidth, spriteHeight);
    addEntity(&testRoom3);
    
    
    
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
    /*
    Entity testRoom2;
    Sprite * resource2 = re->registerSprite("..\\assets\\fiona.png");
    testRoom2.sprite = resource2;
    addEntity(&testRoom2);
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
    // TODO(Michael): figure out what room to draw...
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