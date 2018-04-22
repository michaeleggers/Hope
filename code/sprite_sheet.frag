#version 150

in vec2 texturePosForFrag;
uniform sampler2D tex;
uniform vec4 thing;
out vec4 frag_colour;

void main () 
{
  frag_colour = texture2D(tex, vec2(thing) * texturePosForFrag + thing.zw);
}

