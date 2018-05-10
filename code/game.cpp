
global_var Sprite sprite;
global_var Sprite sprite2;
global_var Spritesheet spriteSheet;
global_var Shader shaders[MAX_SHADERS];

void game_init()
{
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
}

void game_update_and_render(float dt)
{
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