#pragma once

#include "graphics/common.hpp"

// temporary camera class, use event system and ECS later
struct Camera {
    glm::vec3 camera_pos = {0.0f, 0.0f, 0.0f};
    glm::vec3 camera_front = {0.0f, 1.0f, 0.0f};
    glm::vec3 camera_up = {0.0f, 0.0f, 1.0f};
    float camera_speed = 0.05; //4.317f;
    float camera_sensitivity = 0.1f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    float fov = 45.0f;
    bool primary_camera = true;
};
