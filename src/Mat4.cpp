#include "Mat4.hpp"
#include <cmath>

Mat4::Mat4() {
    for (int i = 0; i < 16; ++i) {
        m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
}

Mat4::Mat4(float vals[16]) {
    for (int i = 0; i < 16; ++i) {
        m[i] = vals[i];
    }
}

Mat4::Mat4(const Mat4& other){
    if(this != &other){
        *this = other;
    }
}

Mat4 Mat4::scale(const float s) const {
    Mat4 m;
    for (int i = 0; i < 4; ++i) {
        m[i][i] = s;
    }
    return *this * m;
}

Mat4 Mat4::translate(const Vec3& translation) const {
    Mat4 translationMatrix; // Starts as identity
    translationMatrix[3][0] = translation.x;
    translationMatrix[3][1] = translation.y;
    translationMatrix[3][2] = translation.z;
    return *this * translationMatrix;
}

Mat4 Mat4::rotate(float angle, const Vec3& axis) const {
    Mat4 rot;
    
    float len = sqrt(axis.x*axis.x + axis.y*axis.y + axis.z*axis.z);
    if (len == 0.0f) return *this; 
    float x = axis.x / len;
    float y = axis.y / len;
    float z = axis.z / len;

    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0f - c;

    rot[0][0] = t * x * x + c;
    rot[0][1] = t * x * y + s * z; 
    rot[0][2] = t * x * z - s * y; 
    rot[0][3] = 0.0f;

    rot[1][0] = t * x * y - s * z;
    rot[1][1] = t * y * y + c;
    rot[1][2] = t * y * z + s * x;
    rot[1][3] = 0.0f;

    rot[2][0] = t * x * z + s * y;
    rot[2][1] = t * y * z - s * x;
    rot[2][2] = t * z * z + c;
    rot[2][3] = 0.0f;

    rot[3][0] = 0.0f; rot[3][1] = 0.0f; rot[3][2] = 0.0f; rot[3][3] = 1.0f;

    return *this * rot;
}

Mat4 Mat4::perspective(float fov, float aspect, float near, float far) const {
    Mat4 persp;
    float tanHalfFOV = tan(fov / 2.0f);
    persp[0][0] = 1.0f / (aspect * tanHalfFOV);
    persp[1][1] = 1.0f / (tanHalfFOV);
    persp[2][2] = -(far + near) / (far - near);
    persp[2][3] = -1.0f;
    persp[3][2] = -(2.0f * far * near) / (far - near);
    persp[3][3] = 0.0f;
    return *this * persp;
}

Mat4 Mat4::zero() {
    Mat4 z;
    for (int i = 0; i < 16; ++i) {
        z.m[i] = 0.0f;
    }
    return z;
}

Mat4 Mat4::lookAt(const Vec3& position, const Vec3& target, const Vec3& worldUp) {
    Vec3 zaxis = (position - target).normalized();
    Vec3 xaxis = cross(worldUp, zaxis).normalized();
    Vec3 yaxis = cross(zaxis, xaxis);

    Mat4 view;
    view[0][0] = xaxis.x;
    view[1][0] = xaxis.y;
    view[2][0] = xaxis.z;
    view[3][0] = -dot(xaxis, position);

    view[0][1] = yaxis.x;
    view[1][1] = yaxis.y;
    view[2][1] = yaxis.z;
    view[3][1] = -dot(yaxis, position);

    view[0][2] = zaxis.x;
    view[1][2] = zaxis.y;
    view[2][2] = zaxis.z;
    view[3][2] = -dot(zaxis, position);

    view[0][3] = 0;
    view[1][3] = 0;
    view[2][3] = 0;
    view[3][3] = 1;
    
    return view;
}
