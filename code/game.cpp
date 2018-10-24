#include "game.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


//global_var Sprite sprite;
//global_var Sprite sprite2;
//global_var Spritesheet spriteSheet;
//global_var Shader shaders[MAX_SHADERS]; 

global_var Room gRoomList[1];

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
    re->render(room);
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
    testRoom.background = loadBackground("..\\assets\\azores.png");
    testRoom.background.x = 0;
    testRoom.background.y = 0;
    testRoom.object = loadObject("..\\assets\\fiona.png");
    testRoom.object.x = 15;
    testRoom.object.y = 5;
    gRoomList[0] = testRoom;
    re->loadRooms(&gRoomList[0]);
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
    drawRoom(&gRoomList[0], re);
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