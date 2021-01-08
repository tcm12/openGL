#include "Camera.hpp"
#include <math.h>

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        this->cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction)
        {
        case gps::MOVE_FORWARD:
            this->cameraPosition += speed * cameraFrontDirection;
            break;
        case gps::MOVE_BACKWARD:
            this->cameraPosition -= speed * cameraFrontDirection;
            break;
        case gps::MOVE_RIGHT:
            this->cameraPosition += speed * cameraRightDirection;
            break;
        case gps::MOVE_LEFT:
            this->cameraPosition -= speed * cameraRightDirection;
            break;
        case gps::MOVE_UP:
            this->cameraPosition += speed * cameraUpDirection;
            break;
        case gps::MOVE_DOWN:
            this->cameraPosition -= speed * cameraUpDirection;
            break;
        default:
            break;
        }
        this->cameraTarget = cameraPosition + cameraFrontDirection;
    }

    void Camera::reset() {
        this->cameraPosition = glm::vec3(0.0f, 10.0f, 3.0f);
        this->cameraTarget = glm::vec3(0.0f, 10.0f, -10.0f);
        this->cameraUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 front;
        front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));//can be done directly with cameraFrontDirection.x = ...
        front.y = sin(glm::radians(pitch));
        front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        this->cameraFrontDirection = glm::normalize(front);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        this->cameraTarget = cameraPosition + cameraFrontDirection;
    }
}