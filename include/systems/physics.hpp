#pragma once

#include "ecs/system.hpp"
#include "systems/datatypes.hpp"
#include "utils/logger.hpp"

#include <vector>

class PhysicsSystem : public System {
public:
    void update() {
        for (Archetype* archetype : archetypes) {
            std::vector<Velocity>& velocities = archetype->get_component_vector<Velocity>();

            for (size_t i = 0; i < archetype->size(); i++) {
                glm::vec3& velocity = velocities[i].velocity;

                velocity.z -= 0.08f; // gravity
                velocity.z *= 0.995f; // drag
            }
        }
    }

    void print_info() {
        for (Archetype* archetype : archetypes) {
            std::vector<Transform>& transforms = archetype->get_component_vector<Transform>();
            std::vector<Velocity>& velocities = archetype->get_component_vector<Velocity>();

            for (size_t i = 0; i < archetype->size(); i++) {
                glm::vec3& position = transforms[i].position;
                glm::vec3& velocity = velocities[i].velocity;

                log_debug("Position: %f, %f, %f", position.x, position.y, position.z);
                log_debug("Velocity: %f, %f, %f", velocity.x, velocity.y, velocity.z);
            }
        }
    }
};
