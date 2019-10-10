#ifndef HOPE_MATH_H
#define HOPE_MATH_H

struct v4
{
    float x, y, z, w;
};

struct v3
{
    float x, y, z;
};

struct v2
{
    float x, y;
};


struct mat4
{
    float c[16];
};

mat4 hope_rotate_around_z(float angle);
mat4 hope_translate(float x, float y, float z);
mat4 hope_scale(float x, float y, float z);
void printMat4(mat4 mat);
mat4 mat4x4(mat4 lhs, mat4 rhs);
v3 v3cross(v3 lhs, v3 rhs);
v3 v3normalize(v3 v);
v2 v2normalize(v2 v);
float v2dot(v2 a, v2 b);
float v2length(v2 v);
float v3length(v3 v);
v3 v3add(v3 lhs, v3 rhs);
void hope_create_ortho_matrix(float left, float right, float bottom, float top, float nearVal, float farVal, float * out_m);

#endif