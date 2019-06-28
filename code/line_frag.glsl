
#version 150

uniform vec3 tint;
out vec4 frag_colour;

void main () 
{
    frag_colour = vec4(tint.xyz, 1.0f);
}


