#version 150

in vec2 texturePosForFrag;
uniform sampler2D tex;
uniform vec4 window;
uniform vec3 tint;
uniform int alphaColor;
out vec4 frag_colour;

void main () 
{
    // old API, where each frame a new window into the spritesheet is uploaded to the shader
    //frag_colour = texture2D(tex, window.zw * texturePosForFrag + window.xy);
    
    // new API, just draw the freakin thing. UVs are precomputed on CPU.
    //frag_colour = vec4(tint.xyz, 1.0f) * texture2D(tex, texturePosForFrag);
    vec4 sample = texture2D(tex, texturePosForFrag);
    float blueFromSample = sample.b;
    float alphaColorNorm = alphaColor/255.0f;
    if (blueFromSample == alphaColorNorm) {
        frag_colour = sample * vec4(tint.xyz, 0.f);
    }
    else {
        frag_colour = sample * vec4(tint.xyz, 1.f);
    }
}

