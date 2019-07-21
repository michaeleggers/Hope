#version 150

in vec3 vertex_pos;
in vec2 texture_pos;

out vec2 texturePosForFrag;

uniform mat4 ortho;

void main()
{
    texturePosForFrag = texture_pos;
    gl_Position = ortho * vec4(vertex_pos, 1.0f);
}

