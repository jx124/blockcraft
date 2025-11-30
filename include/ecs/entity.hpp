#pragma once

#include "ecs/common.hpp"

#include <optional>
#include <queue>

// Hands out new EntityIDs.
class EntityManager {
public:
    EntityManager();

    std::optional<EntityID> get_entity();
    void delete_entity(EntityID entity);

private:
    // TODO: implement a ring_buffer if performance of queue is bad
    std::queue<EntityID> available_entities{};
};
