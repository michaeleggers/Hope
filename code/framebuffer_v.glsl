#version 150

in vec3 vertex_pos;
uniform mat4 ortho;
out vec2 UVs;

void main ()
{
    gl_Position = ortho * vec4(vertex_pos, 1.0);
}

