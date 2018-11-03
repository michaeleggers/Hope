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
    /*
    
    // TODO(Michael): get the OpenGL Shit outta here!
    
    // backface/frontface culling (creates less shaders if enabled)
    glEnable (GL_CULL_FACE); // cull face
    glCullFace (GL_BACK); // cull back face
    glFrontFace (GL_CW); // GL_CCW for counter clock-wise
    
    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // enable vsync
    wglSwapIntervalEXT(1);
    
    // multisampling
    glEnable(GL_MULTISAMPLE);
    
    char * shaderAttribs[] = {
        "vertex_pos",
        "texture_pos",
    };
    shaders[SPRITE] = create_shader(
        "..\\code\\sprite.vert",
        "..\\code\\sprite.frag",
        shaderAttribs,
        sizeof(shaderAttribs)/sizeof(*shaderAttribs));
        
    shaders[SPRITE_SHEET] = create_shader(
        "..\\code\\sprite.vert",
        "..\\code\\sprite_sheet.frag",
        shaderAttribs,
        sizeof(shaderAttribs)/sizeof(*shaderAttribs));
        
    sprite = create_sprite("..\\assets\\uv_checkerboard.jpg", &(shaders[SPRITE]));
    sprite2 = create_sprite("..\\assets\\base.png", &(shaders[SPRITE_SHEET]));
    
    spriteSheet = create_spritesheet(&sprite2.texture,
                                     16, 16,
                                     17
                                     );
                                     
    // create and activate ortho matrix
    Rect rect = get_window_dimensions();
    set_ortho(rect.width, rect.height, &shaders[SPRITE]);
    set_ortho(rect.width, rect.height, &shaders[SPRITE_SHEET]);
    
    */
    
    // load "room"
    Room testRoom;
    /*
    testRoom.background = loadBackground("..\\assets\\azores.png");
    testRoom.background.x = 0;
    testRoom.background.y = 0;
    testRoom.object = loadObject("..\\assets\\fiona.png");
    testRoom.object.x = 15;
    testRoom.object.y = 5;
    gRoomList[0] = testRoom;
    re->loadRooms(&gRoomList[0]);
    Sprite * resource = re->registerSprite("..\\assets\\azores.png");
    testRoom.background.refSprite = resource;
    addRoom(&testRoom);
    */
    
    // TODO(Michael): the order of registration matters at the moment,
    // because renderer will draw paint over last pixels...
    // TODO(Michael): later the image file will not define the sprites
    // size. It should be determined by user defined values from asset file.
    // Also, the filename will come from asset file.
    Entity testRoom3;
    int width, height, n;
    unsigned char * testRoom3image = 0;
    Sprite * resource3;
    if (fileExists("..\\assets\\azores.png"))
    {
        testRoom3image = stbi_load("..\\assets\\azores.png", &width, &height, &n, 4);
    }
    else
    {
        // TODO(Michael): load random values into Sprite resource data for debug
        width = 320; // from asset file
        height = 200; // from asset file
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
    resource3 = re->registerSprite("..\\assets\\azores.png", // string just for texture database
                                   testRoom3image, 
                                   width, height);
    testRoom3.sprite = resource3;
    addEntity(&testRoom3);
    
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
    /*
    
    // HACK(Michael): quick way to test if animation works.
    static int frameCount = 0;
    static float posX = 0.0f;
    static float posY = 0.0f;
    static float scaleX = 1.0f;
    static float scaleY = 1.0f;
    
    // render with OpenGL
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    draw_sprite(&sprite, 0, 0);
    draw_frame(&sprite2, &spriteSheet, frameCount % 10,
               sin(posX), sin(posY), 
               40*abs(sin(scaleX)), 40*abs(sin(scaleY)));
    //frameCount++;
    posX += 0.000001f * dt;
    scaleX += 0.000001f * dt;
    scaleY += 0.000001f * dt;
    //posY += 0.6f;
    
    */
    
    // draw room we're in
    //drawRoom(&gRoomList[0], re);
    
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