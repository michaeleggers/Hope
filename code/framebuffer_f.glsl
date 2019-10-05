
#version 150

out vec4 frag_colour;
in vec2 UVs;
uniform sampler2D texture;
void main () 
{
    vec4 fragment = texture2D(texture, UVs);
    frag_colour = fragment;
}


