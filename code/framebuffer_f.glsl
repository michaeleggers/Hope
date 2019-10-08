
#version 150

out vec4 frag_color;
in vec2 texture_pos;
uniform sampler2D texture;
void main () 
{
    vec4 fragment = texture2D(texture, texture_pos);
    frag_color = fragment;
}


