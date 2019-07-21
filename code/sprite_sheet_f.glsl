#version 150

in vec2 texturePosForFrag;
uniform sampler2D tex;
uniform vec4 window;
uniform vec3 tint;
out vec4 frag_colour;

void main () 
{
    // old API, where each frame a new window into the spritesheet is uploaded to the shader
    //frag_colour = texture2D(tex, window.zw * texturePosForFrag + window.xy);
    
    // new API, just draw the freakin thing. UVs are precomputed on CPU.
    frag_colour = vec4(tint.xyz, 1.0f) * texture2D(tex, texturePosForFrag);
    vec4 sample = texture2D(tex, texturePosForFrag);
    if (sample.a > 0.f)
        frag_colour = vec4(tint.xyz, 1.0f) * sample.a;
    else
        frag_colour = vec4(0.f, 1.f, 0.f, 1.f);
    
}

