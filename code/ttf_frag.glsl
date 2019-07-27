#version 150

in vec2 texturePosForFrag;

out vec4 frag_colour;

uniform sampler2D tex;
uniform vec3 tint;

void main () 
{
    vec4 sample = texture2D(tex, texturePosForFrag);
    //if (sample.r > .1f)
    frag_colour = vec4(tint.xyz, sample.r);
    
}


