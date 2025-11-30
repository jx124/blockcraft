#pragma once

#include "ecs/system.hpp"

#include "utils/logger.hpp"

#include <vector>

struct Transform {
    float x = 0;
    float y = 0;
};

struct Velocity {
    float x = 3;
    float y = 4;
};

struct PhysicsSystem : public System {
    void update(float dt) {
        for (Archetype* archetype : archetypes) {
            std::vector<Transform>& positions = archetype->get_component_vector<Transform>();
            std::vector<Velocity>& velocities = archetype->get_component_vector<Velocity>();

            for (size_t i = 0; i < archetype->size(); i++) {
                positions[i].x += dt * velocities[i].x;
                positions[i].y += dt * velocities[i].y;
            }
        }

    }
    void print_info() {
        for (Archetype* archetype : archetypes) {
            std::vector<Transform>& positions = archetype->get_component_vector<Transform>();
            std::vector<Velocity>& velocities = archetype->get_component_vector<Velocity>();

            for (size_t i = 0; i < archetype->size(); i++) {
                log_debug("Position: %f, %f", positions[i].x, positions[i].y);
                log_debug("Velocity: %f, %f", velocities[i].x, velocities[i].y);
            }
        }
    }
};
