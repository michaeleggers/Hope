
global_var Sprite sprite;
global_var Sprite sprite2;
global_var Spritesheet spriteSheet;

void game_init()
{
    // backface/frontface culling (creates less shaders if enabled)
    glEnable (GL_CULL_FACE); // cull face
    glCullFace (GL_BACK); // cull back face
    glFrontFace (GL_CW); // GL_CCW for counter clock-wise
    
    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    char * shaderAttribs[] = {
        "vertex_pos",
        "texture_pos",
    };
    Shader shader = create_shader(
        "..\\code\\sprite.vert",
        "..\\code\\sprite.frag",
        shaderAttribs,
        sizeof(shaderAttribs)/sizeof(*shaderAttribs));
    
    
    sprite = create_sprite("..\\assets\\uv_checkerboard.jpg", &shader);
    sprite2 = create_sprite("..\\assets\\base.png", &shader);
    
    spriteSheet = create_spritesheet(&sprite2.texture,
                                     16, 16,
                                     16
                                     );
    
    // create and activate ortho matrix
    Rect rect = get_window_dimensions();
    set_ortho(rect.width, rect.height);
}

void game_render()
{
    // render with OpenGL
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glUseProgram(shader.shaderProgram);
    //draw_sprite(&sprite);
    //draw_sprite(&sprite2);
    
    //draw_sprite(&sprite2, 0.0f, 0.0f);
    draw_frame(&sprite2, &spriteSheet, 15);
    //draw_sprite(&sprite, -20.0f, 0);
    //glBindVertexArray(quad.vao);
    //glDrawArrays(GL_TRIANGLES, 0, 6);
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