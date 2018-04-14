
global_var Mesh quad;

struct Rect
{
    int width, height;
};


void game_init()
{
    quad = create_quad();
    
    // TODO(Michael): vert and frag are still tightly coupled to impl of create_shader
    char * shaderAttribs[] = {
        "vertex_pos",
        "texture_pos"
    };
    Shader shader = create_shader(
        "..\\code\\sprite.vert",
        "..\\code\\sprite.frag",
        shaderAttribs,
        sizeof(shaderAttribs)/sizeof(*shaderAttribs));
    
    Texture texture = create_texture("..\\assets\\uv_checkerboard.jpg");
    
    glUseProgram(shader.shaderProgram); // TODO(Michael): necessary? -> OMG! YES!!!
    
    // in ogl 4 uniform 0 will do. this is necessary for ogl 3.2
    int tex_loc = glGetUniformLocation(shader.shaderProgram, "tex");
    glUniform1i(tex_loc, 0); // use active texture 0
    
    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    // create ortho matrix
    // TODO(Michael): query windows size from platform layer
    Rect rect = get_window_dimensions();
    float aspectRatio = (float)rect.width / (float)rect.height;
    float orthoMatrix[16] = { };
    ortho(-1.0f * aspectRatio, 1.0f * aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f, orthoMatrix);
    GLuint ortho_loc = glGetUniformLocation(shader.shaderProgram, "ortho");
    glUniformMatrix4fv(ortho_loc, 1, GL_FALSE, orthoMatrix);
}

void game_render()
{
    // render with OpenGL
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glUseProgram(shader.shaderProgram);
    glBindVertexArray(quad.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// NOTE(Michael): move to ogl_render?
void set_ortho(int width, int height)
{
    float aspectRatio = (float)width / (float)height;
    float orthoMatrix[16] = { };
    ortho(-1.0f * aspectRatio, 1.0f * aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f, orthoMatrix);
    GLint shaderID;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shaderID);
    GLuint ortho_loc = glGetUniformLocation(shaderID, "ortho");
    glUniformMatrix4fv(ortho_loc, 1, GL_FALSE, orthoMatrix);
}
