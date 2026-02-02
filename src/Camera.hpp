#pragma once
#include "Vec3.hpp"
#include "Mat4.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum class CameraDirection {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera {
public:
    Vec3 pos, front, up;

    Camera();
    ~Camera() = default;

    Mat4 getViewMatrix() const;
    void move(CameraDirection dir, float distance);
    void rotate(float yaw_offset, float pitch_offset);

private:
    float _yaw;
    float _pitch;
    void updateCameraVectors();
};
