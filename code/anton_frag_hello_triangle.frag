#version 150

in vec2 d;
uniform sampler2D tex;
out vec4 frag_colour;

void main () 
{
  //frag_colour = vec4(d, 1.0f);
  frag_colour = texture2D(tex, vec2(d.x, d.y));
}

