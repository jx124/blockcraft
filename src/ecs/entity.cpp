#include "ecs/entity.hpp"

EntityManager::EntityManager() {
    for (EntityID i = 0; i < MAX_ENTITY_COUNT; i++) {
        available_entities.push(i);
    }
}

std::optional<EntityID> EntityManager::get_entity() {
    if (available_entities.empty()) {
        return std::nullopt;
    }

    std::optional<EntityID> result(available_entities.front());
    available_entities.pop();
    return result;
}

void EntityManager::delete_entity(EntityID entity) {
    available_entities.push(entity);
}
