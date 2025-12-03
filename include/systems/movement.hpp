#pragma once

#include "systems/datatypes.hpp"
#include "ecs/system.hpp"

class MovementSystem : public System {
public:
    void update(float dt) {
        for (Archetype* archetype : archetypes) {
            std::vector<Transform>& transforms = archetype->get_component_vector<Transform>();
            std::vector<Velocity>& velocities = archetype->get_component_vector<Velocity>();
            std::vector<PlayerMovement>& movements = archetype->get_component_vector<PlayerMovement>();

            for (size_t i = 0; i < archetype->size(); i++) {
                glm::vec3& position = transforms[i].position;
                glm::vec3& front = transforms[i].direction;
                glm::vec3& velocity = velocities[i].velocity;
                PlayerMovement& movement = movements[i];

                update_movement(movement);
                
                if (movement.go_forward) {
                    position += movement.movement_speed * front * dt;
                    log_debug("Forward");
                    log_debug("Position: %f, %f, %f", position.x, position.y, position.z);
                    log_debug("Velocity: %f, %f, %f", velocity.x, velocity.y, velocity.z);
                }
                if (movement.go_backward) {
                    position -= movement.movement_speed * front * dt;
                    log_debug("Backward");
                    log_debug("Position: %f, %f, %f", position.x, position.y, position.z);
                    log_debug("Velocity: %f, %f, %f", velocity.x, velocity.y, velocity.z);
                }
                if (movement.go_left) {
                    position -= movement.movement_speed * glm::normalize(glm::cross(front, WORLD_UP)) * dt;
                    log_debug("Left");
                    log_debug("Position: %f, %f, %f", position.x, position.y, position.z);
                    log_debug("Velocity: %f, %f, %f", velocity.x, velocity.y, velocity.z);
                }
                if (movement.go_right) {
                    position += movement.movement_speed * glm::normalize(glm::cross(front, WORLD_UP)) * dt;
                    log_debug("Right");
                    log_debug("Position: %f, %f, %f", position.x, position.y, position.z);
                    log_debug("Velocity: %f, %f, %f", velocity.x, velocity.y, velocity.z);
                }
                if (movement.jump) {
                    velocity += movement.movement_speed * WORLD_UP;
                    log_debug("Jump");
                    log_debug("Position: %f, %f, %f", position.x, position.y, position.z);
                    log_debug("Velocity: %f, %f, %f", velocity.x, velocity.y, velocity.z);
                }

                position += dt * velocity;

                float pitch = movement.pitch;
                float yaw = movement.yaw;

                glm::vec3 new_direction(std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
                        std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
                        std::sin(glm::radians(pitch)));

                front = glm::normalize(new_direction);
            }
        }
    }

private:
    // Read all MovementEvents in queue and update PlayerMovement accordingly.
    void update_movement(PlayerMovement& movement) {
        while (!events.empty()) {
            Event event = std::move(events.front());
            events.pop();

            if (MovementEvent* movement_event = std::get_if<MovementEvent>(&event.event)) {
                switch (movement_event->type) {
                    case MovementEvent::Type::StartMoveForward:
                        movement.go_forward = true;
                        break;
                    case MovementEvent::Type::StopMoveForward:
                        movement.go_forward = false;
                        break;
                    case MovementEvent::Type::StartMoveBackward:
                        movement.go_backward = true;
                        break;
                    case MovementEvent::Type::StopMoveBackward:
                        movement.go_backward = false;
                        break;
                    case MovementEvent::Type::StartMoveLeft:
                        movement.go_left = true;
                        break;
                    case MovementEvent::Type::StopMoveLeft:
                        movement.go_left = false;
                        break;
                    case MovementEvent::Type::StartMoveRight:
                        movement.go_right = true;
                        break;
                    case MovementEvent::Type::StopMoveRight:
                        movement.go_right = false;
                        break;
                    case MovementEvent::Type::Jump:
                        movement.jump = true;
                        break;
                    case MovementEvent::Type::Turn:
                        update_rotation(movement, movement_event->x, movement_event->y);
                        break;
                }
            }
        }
    }

    void update_rotation(PlayerMovement& movement, float x, float y) {
        if (movement.first_mouse) {
            movement.last_x = x;
            movement.last_y = y;
            movement.first_mouse = false;
        }

        float yaw_offset = (movement.last_x - x) * movement.camera_speed;
        float pitch_offset = (movement.last_y - y) * movement.camera_speed;

        movement.last_x = x;
        movement.last_y = y;

        movement.yaw -= yaw_offset;
        movement.pitch += pitch_offset;

        // clamp pitch to [-89.9, 89.9] range
        movement.pitch = movement.pitch < -89.9f
            ? -89.9
            : movement.pitch > 89.9f 
            ? 89.9f
            : movement.pitch;
    }
};
