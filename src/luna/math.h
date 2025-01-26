#pragma once
#include <stdint.h>
#include <stddef.h>

namespace Luna {

constexpr float PI = 3.14159265f;
constexpr float DEG_TO_RAD = PI / 180.0f;

static float abs(float x) { return x < 0 ? -x : x; }
static int abs(int x) { return x < 0 ? -x : x; }

// Clamp function to restrict a value within a range [min, max]
template <typename T>
static T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Min function to return the smaller of two values
template <typename T>
static T min(T a, T b) {
    return (a < b) ? a : b;
}

// Max function to return the larger of two values
template <typename T>
static T max(T a, T b) {
    return (a > b) ? a : b;
}

static float sqrt(float x) {
    if (x < 0) return 0;
    float guess = x;
    for (int i = 0; i < 10; i++) {
        guess = 0.5f * (guess + x / guess);
    }
    return guess;
}

static float sin(float x) {
    x = x - (2 * PI) * static_cast<int>(x / (2 * PI));
    float res = 0;
    float term = x;
    for (int i = 1; i < 10; i++) {
        res += term;
        term *= -x * x / ((2 * i) * (2 * i + 1));
    }
    return res;
}

static float cos(float x) {
    x = x - (2 * PI) * static_cast<int>(x / (2 * PI));
    float res = 1;
    float term = 1;
    for (int i = 1; i < 10; i++) {
        term *= -x * x / ((2 * i - 1) * (2 * i));
        res += term;
    }
    return res;
}

static float tan(float x) {
    return sin(x) / cos(x);
}

struct Vector2 {
    int x, y;
    Vector2(int x = 0, int y = 0) : x(x), y(y) {}
};

struct Vector3 {
    float x, y, z;
    Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct Vector4 {
    float x, y, z, w;
    Vector4(float x = 0, float y = 0, float z = 0, float w = 1) 
        : x(x), y(y), z(z), w(w) {}
};

struct Matrix4x4 {
    float m[4][4] = {
        {1,0,0,0},
        {0,1,0,0},
        {0,0,1,0},
        {0,0,0,1}
    };

    static Matrix4x4 identity() { return Matrix4x4(); }

    static Matrix4x4 multiply(const Matrix4x4& a, const Matrix4x4& b) {
        Matrix4x4 result;
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                result.m[col][row] = 
                    a.m[0][row] * b.m[col][0] +
                    a.m[1][row] * b.m[col][1] +
                    a.m[2][row] * b.m[col][2] +
                    a.m[3][row] * b.m[col][3];
            }
        }
        return result;
    }    
    
    Vector4 operator*(const Vector4& v) const {
        return Vector4{
            m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w,
            m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w,
            m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w,
            m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w
        };
    }

    static Matrix4x4 perspective(float fov, float aspect, float near, float far) {
        Matrix4x4 result;
        float tanHalfFov = tan(fov / 2.0f);

        result.m[0][0] = 1.0f / (aspect * tanHalfFov);
        result.m[1][1] = 1.0f / tanHalfFov;
        result.m[2][2] = (far + near) / (near - far);
        result.m[2][3] = (2.0f * far * near) / (near - far);
        result.m[3][2] = -1.0f; // Critical fix: sets w to -z_view
        result.m[3][3] = 0.0f;

        return result;
    }

    static Matrix4x4 translate(float x, float y, float z) {
        Matrix4x4 mat;
        mat.m[3][0] = x;
        mat.m[3][1] = y;
        mat.m[3][2] = z;
        return mat;
    }
    static Matrix4x4 rotateX(float angle) {
        Matrix4x4 mat;
        float c = cos(angle);
        float s = sin(angle);
        mat.m[1][1] = c;
        mat.m[1][2] = s;
        mat.m[2][1] = -s;
        mat.m[2][2] = c;
        return mat;
    }

    static Matrix4x4 rotateY(float angle) {
        Matrix4x4 mat;
        float c = cos(angle);
        float s = sin(angle);
        mat.m[0][0] = c;
        mat.m[0][2] = -s;
        mat.m[2][0] = s;
        mat.m[2][2] = c;
        return mat;
    }

    static Matrix4x4 rotateZ(float angle) {
        Matrix4x4 mat;
        float c = cos(angle);
        float s = sin(angle);
        mat.m[0][0] = c;
        mat.m[0][1] = s;
        mat.m[1][0] = -s;
        mat.m[1][1] = c;
        return mat;
    }

    static Matrix4x4 scale(float x, float y, float z) {
        Matrix4x4 mat;
        mat.m[0][0] = x;
        mat.m[1][1] = y;
        mat.m[2][2] = z;
        return mat;
    }
};


} // namespace Luna