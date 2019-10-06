#version 150

//in vec3 vertex_pos;
uniform mat4 ortho;
out vec2 UVs;

vec4 vertex_pos[] = vec4[](
vec4(-1,-1,0,1),
vec4(1,-1,0,1),
vec4(1,1,0,1),

vec4(1,1,0,1),
vec4(-1,1,0,1),
vec4(-1,-1,0,1)
);

void main ()
{
    int vertex_id = gl_VertexID;
    UVs = vertex_pos[vertex_id].xy;
    gl_Position = ortho * vertex_pos[vertex_id];
}

