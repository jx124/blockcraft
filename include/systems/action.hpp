#pragma once

#include "blocks/chunk_manager.hpp"
#include "systems/camera.hpp"
#include "systems/datatypes.hpp"
#include "ecs/system.hpp"

class ActionSystem : public System {
public:
    void update(ChunkManager& chunk_manager, CameraSystem& camera_system) {
        for (Archetype* archetype : archetypes) {
            std::vector<PlayerAction>& actions = archetype->get_component_vector<PlayerAction>();

            for (size_t i = 0; i < archetype->size(); i++) {
                glm::vec3 position = camera_system.camera_position();
                glm::vec3 front = camera_system.camera_direction();
                PlayerAction& action = actions[i];

                update_action(action);

                if (action.left_click) {
                    log_debug("Left click");
                    log_debug("Position: %f, %f, %f", position.x, position.y, position.z);
                    log_debug("Direction: %f, %f, %f", front.x, front.y, front.z);
                    std::optional<glm::vec3> block = chunk_manager.cast_ray(position, front, false);
                    if (block) {
                        chunk_manager.add_event(Event::make_event(BlockEvent::Type::Break, *block));
                    }
                }
                if (action.right_click) {
                    log_debug("Right click");
                    log_debug("Position: %f, %f, %f", position.x, position.y, position.z);
                    log_debug("Direction: %f, %f, %f", front.x, front.y, front.z);
                    std::optional<glm::vec3> block = chunk_manager.cast_ray(position, front, true);
                    if (block) {
                        chunk_manager.add_event(Event::make_event(BlockEvent::Type::Place, *block, Block{ Block::Type::DIRT }));
                    }
                }
            }
        }
    }

private:
    void update_action(PlayerAction& action) {
        while (!events.empty()) {
            Event event = std::move(events.front());
            events.pop();

            if (ActionEvent* action_event = std::get_if<ActionEvent>(&event.event)) {
                switch (action_event->type) {
                    case ActionEvent::Type::StartLeftClick:
                        action.left_click = true;
                        break;
                    case ActionEvent::Type::StopLeftClick:
                        action.left_click = false;
                        break;
                    case ActionEvent::Type::StartRightClick:
                        action.right_click = true;
                        break;
                    case ActionEvent::Type::StopRightClick:
                        action.right_click = false;
                        break;
                }
            }
        }
    }
};
