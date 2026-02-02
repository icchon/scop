#pragma once
#include <array>
#include "Vec3.hpp"

class Mat4 {
public:
    float m[16];
    Mat4();
    Mat4(float vals[16]);
    Mat4(const Mat4& other);
    ~Mat4() = default;
    Mat4& operator=(const Mat4& other) {
        if (this != &other) {
            for (int i = 0; i < 16; ++i) {
                m[i] = other.m[i];
            }
        }
        return *this;
    }
    float* operator[](int col){
        return &m[col * 4];
    }
    const float* operator[](int col) const {
        return &m[col * 4];
    }
    Mat4 operator+(const Mat4& other) const {
        Mat4 result;
        for (int i = 0; i < 16; ++i) {
            result.m[i] = m[i] + other.m[i];
        }
        return result;
    }
    Mat4 operator*(const Mat4& other) const {
    Mat4 res;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int i = 0; i < 4; ++i) {
                sum += (*this)[i][row] * other[col][i];
            }
            res[col][row] = sum;
        }
    }
    return res;
    }
    Mat4 translate(const Vec3& translation) const;
    Mat4 scale(const float s) const;
    Mat4 rotate(float angle, const Vec3& axis) const;
    Mat4 perspective(float fov, float aspect, float near, float far) const;
    static Mat4 zero();
    static Mat4 lookAt(const Vec3& position, const Vec3& target, const Vec3& worldUp);
};

