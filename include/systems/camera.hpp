#pragma once

#include "ecs/system.hpp"
#include "systems/datatypes.hpp"

class CameraSystem : public System {
public:
    void register_primary_camera() {
        for (Archetype* archetype : archetypes) {
            std::vector<Transform>& transforms = archetype->get_component_vector<Transform>();
            std::vector<Camera>& cameras = archetype->get_component_vector<Camera>();

            for (size_t i = 0; i < archetype->size(); i++) {
                if (!cameras[i].is_primary_camera) {
                    continue;
                }
                primary_camera = &cameras[i];
                position = &transforms[i].position;
                direction = &transforms[i].direction;
            }
        }
    };

    glm::mat4 view() const {
        debug_assert(primary_camera && position && direction, "Did not register primary camera for view matrix");

        return glm::lookAt(*position + primary_camera->offset,
                *position + *direction + primary_camera->offset,
                WORLD_UP);
    }

    glm::mat4 projection() const {
        debug_assert(primary_camera && position && direction, "Did not register primary camera for projection matrix");

        return glm::perspective(primary_camera->field_of_view_y,
                primary_camera->aspect_ratio,
                primary_camera->near_plane,
                primary_camera->far_plane);
    }

private:
    Camera* primary_camera{};
    glm::vec3* position{};
    glm::vec3* direction{};
};
