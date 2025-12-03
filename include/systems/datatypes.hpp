#pragma once

#include "graphics/common.hpp"

constexpr glm::vec3 WORLD_UP(0.0f, 0.0f, 1.0f);

struct Transform {
    glm::vec3 position{};
    glm::vec3 direction{0.0f, 1.0f, 0.0f}; // normalized
};

struct Velocity {
    glm::vec3 velocity{};
};

struct PlayerMovement {
    float movement_speed = 5.0f;
    float camera_speed = 0.1f;
    float last_x{};
    float last_y{};
    float pitch{};
    float yaw{};
    bool first_mouse = true;

    bool go_forward{};
    bool go_backward{};
    bool go_left{};
    bool go_right{};
    bool jump{};
    bool landed{};
};

struct Camera {
    float field_of_view_y = glm::radians(45.0f);
    float aspect_ratio = 1920.0f / 1200.0f; // TODO: add resize event that changes the aspect ratio
    float near_plane = 0.1f;
    float far_plane = 1000.0f;
    glm::vec3 offset{};  // offset from the player, change to generic view matrix if needed
    bool is_primary_camera = true;
};
