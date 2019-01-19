#version 150

in vec3 vertex_pos;
in vec3 normals;
in vec2 UVs;
uniform mat4 projectionMat;
uniform mat4 modelMat;

out vec3 color;

void main ()
{
    gl_Position = projectionMat * modelMat * vec4(vertex_pos, 1.0);
    color = vertex_pos;
}
