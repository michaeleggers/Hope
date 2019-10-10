#include <math.h>
#include "hope_math.h"

#define PI 3.141592f

v3 v3add(v3 lhs, v3 rhs)
{
    return {
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z
    };
};

float v3length(v3 v)
{
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

float v2length(v2 v)
{
    return sqrt(v.x*v.x + v.y*v.y);
}

v3 v3normalize(v3 v)
{
    float length = v3length(v);
    return {
        v.x / length,
        v.y / length,
        v.z / length
    };
}

v3 v3cross(v3 lhs, v3 rhs)
{
    return {
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x
    };
}

mat4 mat4x4(mat4 lhs, mat4 rhs)
{
    mat4 result = {};
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            for (int inner = 0; inner < 4; inner++)
            {
                result.c[4*row+col] += lhs.c[4*inner+col] * rhs.c[4*row+inner];
            }
        }
    }
    return result;
}

void printMat4(mat4 mat)
{
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            printf("%f ", mat.c[4*col + row]);
        }
        printf("\n");
    }
}

mat4 hope_scale(float x, float y, float z)
{
    mat4 result = {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    };
    return result;
}

mat4 hope_translate(float x, float y, float z)
{
    mat4 result = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1
    };
    return result;
}

// NOTE(Michael): http://www.kwon3d.com/theory/transform/rot.html
mat4 hope_rotate_around_z(float angle)
{
    float angleInRad  = (PI*angle) / 180.f;
    mat4 result = {
        cosf(angleInRad), sinf(angleInRad), 0, 0,
        -sinf(angleInRad), cosf(angleInRad), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    return result;
}

void hope_create_ortho_matrix(float left, float right, float bottom, float top, float nearVal, float farVal, float * out_m)
{
    float tx = -((right + left) / (right - left));
    float ty = -((top + bottom) / (top - bottom));
    float tz = -((farVal + nearVal) / (farVal - nearVal));
    float x = 2.0f / (right - left);
    float y = 2.0f / (top - bottom);
    float z = -2.0f / (farVal - nearVal);
    
    float src[16] = {
        x, 0.0f, 0.0f, 0.0f,
        0.0f, y, 0.0f, 0.0f,
        0.0f, 0.0f, z, 0.0f,
        tx, ty, tz, 1.0f
    };
    memcpy(out_m, src, 16 * sizeof(float));
};
