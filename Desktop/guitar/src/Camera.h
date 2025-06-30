#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{
public:
    Camera(float width, float height);

    void setPosition(const glm::vec3 &position);
    void setTarget(const glm::vec3 &target);
    void setFov(float fovDegrees);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::vec3 getPosition() const { return position_; }
    glm::vec3 getTarget() const { return target_; }

    void handleMouseMotion(int deltaX, int deltaY);
    void handleMouseWheel(int delta);
    void updateAspectRatio(float width, float height);

    // Ray casting for 3D click detection
    glm::vec3 screenToWorldRay(int screenX, int screenY, int screenWidth, int screenHeight);

private:
    glm::vec3 position_;
    glm::vec3 target_;
    glm::vec3 up_;

    float fov_;
    float aspectRatio_;
    float nearPlane_;
    float farPlane_;

    void updateViewMatrix();
    glm::mat4 viewMatrix_;
    glm::mat4 projectionMatrix_;
};