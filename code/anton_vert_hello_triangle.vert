#version 150

in vec3 vertex_pos;
in vec2 texture_pos;
uniform mat4 ortho;

out vec2 d;

void main ()
{
  d = texture_pos;
  gl_Position = ortho * vec4(vertex_pos, 1.0);
}