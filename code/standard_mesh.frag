#version 150

out vec4 frag_colour;
in vec3 color;

void main () 
{
  frag_colour = vec4(0.5f*vec3(color)+0.5f, 1.0f);
}





