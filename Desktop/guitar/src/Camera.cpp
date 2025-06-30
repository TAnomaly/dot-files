#include "Camera.h"
#include <algorithm>

Camera::Camera(float width, float height)
    : position_(0.0f, 0.0f, 3.0f), target_(0.0f, 0.0f, 0.0f), up_(0.0f, 1.0f, 0.0f), fov_(45.0f), aspectRatio_(width / height), nearPlane_(0.1f), farPlane_(100.0f)
{

    updateViewMatrix();
    projectionMatrix_ = glm::perspective(glm::radians(fov_), aspectRatio_, nearPlane_, farPlane_);
}

void Camera::setPosition(const glm::vec3 &position)
{
    position_ = position;
    updateViewMatrix();
}

void Camera::setTarget(const glm::vec3 &target)
{
    target_ = target;
    updateViewMatrix();
}

void Camera::setFov(float fovDegrees)
{
    fov_ = fovDegrees;
    projectionMatrix_ = glm::perspective(glm::radians(fov_), aspectRatio_, nearPlane_, farPlane_);
}

glm::mat4 Camera::getViewMatrix() const
{
    return viewMatrix_;
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return projectionMatrix_;
}

void Camera::handleMouseMotion(int deltaX, int deltaY)
{
    // Orbit camera around target
    const float sensitivity = 0.01f;

    glm::vec3 direction = position_ - target_;
    float radius = glm::length(direction);

    // Calculate spherical coordinates
    float theta = atan2(direction.z, direction.x);
    float phi = acos(direction.y / radius);

    // Update angles
    theta -= deltaX * sensitivity;
    phi += deltaY * sensitivity;

    // Constrain phi to avoid flipping
    phi = std::max(0.1f, std::min(phi, 3.14f - 0.1f));

    // Convert back to cartesian
    position_.x = target_.x + radius * sin(phi) * cos(theta);
    position_.y = target_.y + radius * cos(phi);
    position_.z = target_.z + radius * sin(phi) * sin(theta);

    updateViewMatrix();
}

void Camera::handleMouseWheel(int delta)
{
    // Zoom in/out
    glm::vec3 direction = glm::normalize(target_ - position_);
    float distance = glm::length(target_ - position_);

    distance -= delta * 0.1f;
    distance = std::max(0.5f, std::min(distance, 10.0f));

    position_ = target_ - direction * distance;
    updateViewMatrix();
}

void Camera::updateAspectRatio(float width, float height)
{
    aspectRatio_ = width / height;
    projectionMatrix_ = glm::perspective(glm::radians(fov_), aspectRatio_, nearPlane_, farPlane_);
}

glm::vec3 Camera::screenToWorldRay(int screenX, int screenY, int screenWidth, int screenHeight)
{
    // Convert screen coordinates to normalized device coordinates
    float x = (2.0f * screenX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * screenY) / screenHeight;

    // Create ray in clip space
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

    // Convert to eye space
    glm::vec4 rayEye = glm::inverse(projectionMatrix_) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Convert to world space
    glm::vec3 rayWorld = glm::vec3(glm::inverse(viewMatrix_) * rayEye);
    return glm::normalize(rayWorld);
}

void Camera::updateViewMatrix()
{
    viewMatrix_ = glm::lookAt(position_, target_, up_);
}