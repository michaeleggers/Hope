#version 150

in vec2 texturePosForFrag;
uniform sampler2D tex;
uniform vec2 thing;
out vec4 frag_colour;

void main () 
{
  frag_colour = texture2D(tex, thing * texturePosForFrag + 0*vec2(thing.x, 0.0f));
}

