#version 150

//in vec3 vertex_pos;
out vec2 texture_pos;

vec4 vertex_pos[] = vec4[](
vec4(-1,-1,0,1),
vec4(1,-1,0,1),
vec4(1,1,0,1),

vec4(1,1,0,1),
vec4(-1,1,0,1),
vec4(-1,-1,0,1)
);

vec2 UVs[] = vec2[](
vec2(0,0),
vec2(1,0),
vec2(1,1),

vec2(1,1),
vec2(0,1),
vec2(0,0)
);

void main ()
{
    int vertex_id = gl_VertexID;
    texture_pos = UVs[vertex_id].xy;
    gl_Position = vertex_pos[vertex_id];
}

