#version 150

in vec3 vertex_pos;
in vec3 normal;
in vec2 texture_pos;
uniform mat4 ortho;
uniform mat4 model;

out vec2 texturePosForFrag;

void main ()
{
    texturePosForFrag = texture_pos;
    //gl_Position = ortho * model * vec4(vertex_pos, 1.0);
    gl_Position = ortho * vec4(vertex_pos, 1.0);
}