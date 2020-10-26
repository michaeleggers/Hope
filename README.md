# Hope
Small-ish game engine featuring 2D Batch rendering, Keyboard/Mouse/XBox360 Input and the beginning of a immediate-mode GUI API.
The small JSON-Parser I wrote is able to parse Aseprite-JSON output to
extract the position of individual frame out of a spritesheet.

Rendering is done using OpenGL, platform specific things directly using Win32 API.
Uses stb_image to load images from disk and stb_truetype to load TTF files and
rasterize them into a texture.

![alt text](https://github.com/michaeleggers/Hope/blob/master/readmeassets/indyanimation.gif "Hope Demo")





