#include "Camera.hpp"
#include "Vec3.hpp"
#include <cmath>

Camera::Camera() 
    : pos(0.0f, 0.0f, 5.0f), 
      front(0.0f, 0.0f, -1.0f), 
      up(0.0f, 1.0f, 0.0f),
      _yaw(-90.0f),
      _pitch(0.0f) {
    updateCameraVectors();
}

Mat4 Camera::getViewMatrix() const {
    return Mat4::lookAt(pos, pos + front, up);
}

void Camera::move(CameraDirection dir, float distance) {
    if (dir == CameraDirection::FORWARD) {
        pos += front * distance;
    }
    if (dir == CameraDirection::BACKWARD) {
        pos -= front * distance;
    }
    if (dir == CameraDirection::LEFT) {
        pos -= cross(front, up).normalized() * distance;
    }
    if (dir == CameraDirection::RIGHT) {
        pos += cross(front, up).normalized() * distance;
    }
}

void Camera::rotate(float yaw_offset, float pitch_offset) {
    _yaw += yaw_offset;
    _pitch += pitch_offset;

    if (_pitch > 89.0f) {
        _pitch = 89.0f;
    }
    if (_pitch < -89.0f) {
        _pitch = -89.0f;
    }
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    Vec3 new_front;
    new_front.x = cos(_yaw * M_PI / 180.0) * cos(_pitch * M_PI / 180.0);
    new_front.y = sin(_pitch * M_PI / 180.0);
    new_front.z = sin(_yaw * M_PI / 180.0) * cos(_pitch * M_PI / 180.0);
    front = new_front.normalized();
}
