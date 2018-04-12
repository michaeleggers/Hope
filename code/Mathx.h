#ifndef MATHX_H
#define MATHX_H

#include <vector>
#include <iostream>

struct vec4
{
    vec4() { values[0] = values[1] = values[2] = values[3] = 0; };
    
    vec4(float x, float y, float z, float w)
    {
        values[0] = x;
        values[1] = y;
        values[2] = z;
        values[3] = w;
    }
    
    float values[4];
    
    float& operator[](int index)
    {
        return values[index];
    }
    
    float const & operator[](int index) const
    {
        return values[index];
    }
    
    vec4 operator-(vec4 const & rhs)
    {
        return { values[0] - rhs.values[0]
                , values[1] - rhs.values[1]
                , values[2] - rhs.values[2]
                , values[3] - rhs.values[3] };
    }
    
    friend std::ostream& operator<<(std::ostream& os, vec4 const & rhs)
    {
        os << rhs.values[0] << std::endl
            << rhs.values[1] << std::endl
            << rhs.values[2] << std::endl
            << rhs.values[3] << std::endl;
        return os;
    }
};

struct vec3
{
    vec3(float x, float y, float z) :
    x_(x),
    y_(y),
    z_(z)
    {}
    
    vec3() : x_(0), y_(0), z_(0) {}
    
    float x_, y_, z_;
    
    vec3 operator-(vec3 const & rhs)
    {
        return vec3{ x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_ };
    }
    
    vec3 operator+(vec3 const & rhs)
    {
        return { x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_ };
    }
    
    friend vec3 operator*(float const & scalar, vec3 const & rhs)
    {
        return { rhs.x_ * scalar, rhs.y_ * scalar, rhs.z_ * scalar };
    }
    
    friend std::ostream& operator<<(std::ostream& os, vec3 const & rhs)
    {
        os << rhs.x_ << std::endl << rhs.y_ << std::endl << rhs.z_ << std::endl;
        return os;
    }
};

vec3 normalize(vec3 const & v)
{
    float length = sqrt(v.x_ * v.x_ + v.y_ * v.y_ + v.z_ * v.z_);
    return { v.x_ / length, v.y_ / length, v.z_ / length };
}

float dot(vec3 const & lhs, vec3 const & rhs)
{
    return 
    {
        lhs.x_ * rhs.x_ + lhs.y_ * rhs.y_ + lhs.z_ * rhs.z_
    };
}

vec3 cross(vec3 const & lhs, vec3 const & rhs)
{
    return
    {
        lhs.y_ * rhs.z_ - lhs.z_ * rhs.y_,
        lhs.z_ * rhs.x_ - lhs.x_ * rhs.z_,
        lhs.x_ * rhs.y_ - lhs.y_ * rhs.x_
    };
}

struct mat4x4
{
    vec4 columns[4];
    
    vec4& operator[](int index)
    {
        return columns[index];
    }
    
    vec4 const & operator[](int index) const
    {
        return columns[index];
    }
    
    friend std::ostream& operator<<(std::ostream& os, mat4x4 const & rhs)
    {
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                os << rhs[col][row] << " ";
            }
            os << std::endl;
        }
        return os;
    }
};

// vector * matrix multiplication
vec4 preVecMult(vec4 const & v, mat4x4 const & m)
{
    return vec4{
        v[0] * m[0][0] + v[1] * m[0][1] + v[2] * m[0][2] + v[3] * m[0][3],
        v[0] * m[1][0] + v[1] * m[1][1] + v[2] * m[1][2] + v[3] * m[1][3],
        v[0] * m[2][0] + v[1] * m[2][1] + v[2] * m[2][2] + v[3] * m[2][3],
        v[0] * m[3][0] + v[1] * m[3][1] + v[2] * m[3][2] + v[3] * m[3][3]
    };
}

vec4 postVecMult(mat4x4 const & m, vec4 const & v)
{
    return vec4{
        { m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0] * v[3] },
        { m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1] * v[3] },
        { m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2] * v[3] },
        { m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3] * v[3] },
    };
}

mat4x4 mat4x4Mult(mat4x4 const & lhs, mat4x4 const & rhs)
{
    mat4x4 product;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            for (int inner = 0; inner < 4; inner++) 
            {
                product[col][row] += lhs[inner][row] * rhs[col][inner];
            }
        }
    }
    return product;
};

mat4x4 LookAtRH(vec3 eye, vec3 lookAt, vec3 up)
{
    vec3 viewDir = normalize(lookAt - eye);    // The "forward" vector.
    vec3 viewSide; // The "right" vector.
    vec3 viewUp;     // The "up" vector.
    
    // Gram-Schmid orthogonalization (see 236, Van Verth, Bishop)
    viewUp = normalize(up - dot(up, viewDir) * viewDir);
    viewSide = cross(viewUp, viewDir);
    
    // Create a 4x4 view matrix from the right, up, forward and eye position vectors
    mat4x4 viewMatrix =
    {
        vec4{ viewSide.x_,            viewUp.x_,            viewDir.x_,       0 },
        vec4{ viewSide.y_,            viewUp.y_,            viewDir.y_,       0 },
        vec4{ viewSide.z_,            viewUp.z_,            viewDir.z_,       0 },
        vec4{ -dot(viewSide, eye), -dot(viewUp, eye), -dot(viewDir, eye),  1 }
    };
    
    return viewMatrix;
}

void ortho(float left, float right, float bottom, float top, float nearVal, float farVal, float * m)
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
    memcpy(m, src, 16 * sizeof(float));
};


#endif