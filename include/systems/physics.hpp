#pragma once

#include "ecs/system.hpp"
#include "systems/datatypes.hpp"
#include "utils/logger.hpp"

#include <vector>

struct PhysicsSystem : public System {
    void update(float dt) {
        for (Archetype* archetype : archetypes) {
            std::vector<Transform>& transforms = archetype->get_component_vector<Transform>();
            std::vector<Velocity>& velocities = archetype->get_component_vector<Velocity>();

            for (size_t i = 0; i < archetype->size(); i++) {
                glm::vec3& position = transforms[i].position;
                glm::vec3& velocity = velocities[i].velocity;

                position += velocity * dt;
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
