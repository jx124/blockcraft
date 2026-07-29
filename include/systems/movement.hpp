#pragma once

#include "blocks/chunk_manager.hpp"
#include "systems/datatypes.hpp"
#include "ecs/system.hpp"

class MovementSystem : public System {
public:
    void update(float dt, ChunkManager& chunk_manager) {
        for (Archetype* archetype : archetypes) {
            std::vector<Transform>& transforms = archetype->get_component_vector<Transform>();
            std::vector<Velocity>& velocities = archetype->get_component_vector<Velocity>();
            std::vector<PlayerMovement>& movements = archetype->get_component_vector<PlayerMovement>();
            std::vector<AABB>& AABBs = archetype->get_component_vector<AABB>();

            for (size_t i = 0; i < archetype->size(); i++) {
                glm::vec3& position = transforms[i].position;
                glm::vec3& front = transforms[i].direction;
                glm::vec3& velocity = velocities[i].velocity;
                PlayerMovement& movement = movements[i];

                update_movement(movement);
                glm::vec3 movement_velocity{};

                if (movement.go_forward) {
                    movement_velocity += movement.movement_speed * glm::normalize(front * glm::vec3(1.0f, 1.0f, 0.0f));
                }
                if (movement.go_backward) {
                    movement_velocity -= movement.movement_speed * glm::normalize(front * glm::vec3(1.0f, 1.0f, 0.0f));
                }
                if (movement.go_left) {
                    movement_velocity -= movement.movement_speed * glm::normalize(glm::cross(front, WORLD_UP));
                }
                if (movement.go_right) {
                    movement_velocity += movement.movement_speed * glm::normalize(glm::cross(front, WORLD_UP));
                }
                if (movement.jump) {
                    velocity += movement.jump_speed * WORLD_UP;
                    movement.jump = false;
                    movement.landed = false;
                }

                bool hit_ground = move_and_collide_z(dt, AABBs[i], position, velocity, chunk_manager);
                if (hit_ground) {
                    movement.landed = true;
                }
                move_and_collide_x(dt, AABBs[i], position, movement_velocity, chunk_manager);
                move_and_collide_y(dt, AABBs[i], position, movement_velocity, chunk_manager);

                chunk_manager.update(position);

                float pitch = movement.pitch;
                float yaw = movement.yaw;

                glm::vec3 new_direction(std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
                        std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
                        std::sin(glm::radians(pitch)));

                front = glm::normalize(new_direction);
            }
        }
    }

    void reset_first_mouse() {
        for (Archetype* archetype : archetypes) {
            std::vector<PlayerMovement>& movements = archetype->get_component_vector<PlayerMovement>();

            for (size_t i = 0; i < archetype->size(); i++) {
                movements[i].first_mouse = true;
            }
        }
    }

    static void move_and_collide_x(float dt, const AABB& aabb, glm::vec3& position, glm::vec3& velocity, ChunkManager& chunk_manager) {
        if (velocity.x == 0.0f) {
            return;
        }

        AABB box = aabb.get_translated_copy(position);
        glm::ivec3 min_voxel = glm::floor(box.min);
        glm::ivec3 max_voxel = glm::floor(box.max);
        float dx = velocity.x * dt;

        if (dx > 0) {
            int first_x = glm::floor(box.max.x);
            int last_x = glm::floor(box.max.x + dx);
            bool blocked = false;

            for (int x = first_x + 1; x <= last_x && !blocked; x++) {
                for (int y = min_voxel.y; y <= max_voxel.y && !blocked; y++) {
                    for (int z = min_voxel.z; z <= max_voxel.z; z++) {
                        if (chunk_manager.is_block_solid({x, y, z})) {
                            dx = box.max.x - x;
                            blocked = true;
                            velocity.x = 0.0f;
                            break;
                        }
                    }
                }
            }
        } else {
            int first_x = glm::floor(box.min.x);
            int last_x = glm::floor(box.min.x + dx);
            bool blocked = false;

            for (int x = first_x - 1; x >= last_x && !blocked; x--) {
                for (int y = min_voxel.y; y <= max_voxel.y && !blocked; y++) {
                    for (int z = min_voxel.z; z <= max_voxel.z; z++) {
                        if (chunk_manager.is_block_solid({x, y, z})) {
                            dx = (x + 1.0f) - box.min.x;
                            blocked = true;
                            velocity.x = 0.0f;
                            break;
                        }
                    }
                }
            }
        }

        position.x += dx;
    }

    static void move_and_collide_y(float dt, const AABB& aabb, glm::vec3& position, glm::vec3& velocity, ChunkManager& chunk_manager) {
        if (velocity.y == 0.0f) {
            return;
        }

        AABB box = aabb.get_translated_copy(position);
        glm::ivec3 min_voxel = glm::floor(box.min);
        glm::ivec3 max_voxel = glm::floor(box.max);
        float dy = velocity.y * dt;

        if (dy > 0) {
            int first_y = glm::floor(box.max.y);
            int last_y = glm::floor(box.max.y + dy);
            bool blocked = false;

            for (int y = first_y + 1; y <= last_y && !blocked; y++) {
                for (int z = min_voxel.z; z <= max_voxel.z && !blocked; z++) {
                    for (int x = min_voxel.x; x <= max_voxel.x; x++) {
                        if (chunk_manager.is_block_solid({x, y, z})) {
                            dy = box.max.y - y;
                            blocked = true;
                            velocity.y = 0.0f;
                            break;
                        }
                    }
                }
            }
        } else {
            int first_y = glm::floor(box.min.y);
            int last_y = glm::floor(box.min.y + dy);
            bool blocked = false;

            for (int y = first_y - 1; y >= last_y && !blocked; y--) {
                for (int z = min_voxel.z; z <= max_voxel.z && !blocked; z++) {
                    for (int x = min_voxel.x; x <= max_voxel.x; x++) {
                        if (chunk_manager.is_block_solid({x, y, z})) {
                            dy = (y + 1.0f) - box.min.y;
                            blocked = true;
                            velocity.y = 0.0f;
                            break;
                        }
                    }
                }
            }
        }

        position.y += dy;
    }

    static bool move_and_collide_z(float dt, const AABB& aabb, glm::vec3& position, glm::vec3& velocity, ChunkManager& chunk_manager) {
        if (velocity.z == 0.0f) {
            return true;
        }

        AABB box = aabb.get_translated_copy(position);
        glm::ivec3 min_voxel = glm::floor(box.min);
        glm::ivec3 max_voxel = glm::floor(box.max);
        float dz = velocity.z * dt;
        bool blocked = false;

        if (dz > 0) {
            int first_z = glm::floor(box.max.z);
            int last_z = glm::floor(box.max.z + dz);

            for (int z = first_z + 1; z <= last_z && !blocked; z++) {
                for (int x = min_voxel.x; x <= max_voxel.x && !blocked; x++) {
                    for (int y = min_voxel.y; y <= max_voxel.y; y++) {
                        if (chunk_manager.is_block_solid({x, y, z})) {
                            dz = box.max.z - z;
                            blocked = true;
                            velocity.z = 0.0f;
                            break;
                        }
                    }
                }
            }
        } else {
            int first_z = glm::floor(box.min.z);
            int last_z = glm::floor(box.min.z + dz);

            for (int z = first_z - 1; z >= last_z && !blocked; z--) {
                for (int x = min_voxel.x; x <= max_voxel.x && !blocked; x++) {
                    for (int y = min_voxel.y; y <= max_voxel.y; y++) {
                        if (chunk_manager.is_block_solid({x, y, z})) {
                            dz = (z + 1.0f) - box.min.z;
                            blocked = true;
                            velocity.z = 0.0f;
                            break;
                        }
                    }
                }
            }
        }

        position.z += dz;
        return blocked;
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
                        if (movement.landed) {
                            movement.jump = true;
                        }
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
