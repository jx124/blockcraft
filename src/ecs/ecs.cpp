#include "ecs/ecs.hpp"


std::optional<EntityID> EntityComponentSystem::create_entity() {
    return entity_manager.get_entity();
}

void EntityComponentSystem::delete_entity(EntityID entity) {
    debug_assert(entity_archetype_map.contains(entity), "Deleting a non-existent entity");

    entity_manager.delete_entity(entity);

    ArchetypeID archetype_id = entity_archetype_map.at(entity);
    archetypes.at(archetype_id)->delete_entity(entity);

    entity_archetype_map.erase(entity);
}

void EntityComponentSystem::clear_unused_archetypes() {
    std::unordered_set<ArchetypeID> used_archetypes;
    for (const auto& [entity, archetype_id] : entity_archetype_map) {
        used_archetypes.insert(archetype_id);
    }

    std::unordered_set<ArchetypeID> unused_archetypes;
    for (auto& [archetype_id, archetype] : archetypes) {
        if (used_archetypes.contains(archetype_id)) {
            continue;
        }
        unused_archetypes.insert(archetype_id);
        log_debug("Unused archetype %b", archetype_id);
    }

    for (auto& [_, system] : systems) {
        // we have not released our unique_ptrs yet, so the system archetype pointers are still valid
        system->delete_unused_archetypes(unused_archetypes);
    }

    for (ArchetypeID unused_archetype : unused_archetypes) {
        if (archetypes.contains(unused_archetype)) {
            archetypes.at(unused_archetype)->clear();
            archetypes.erase(unused_archetype);
        }
    }
}

void EntityComponentSystem::register_component_type(const char* type_name) {
    debug_assert(!type_component_map.contains(type_name), "Component already registered");
    debug_assert(current_component_id < MAX_COMPONENT_TYPES, "Too many components registered");
    
    type_component_map.insert({type_name, current_component_id});
    current_component_id++;
}

void EntityComponentSystem::create_new_archetype(ArchetypeID new_archetype_id) {
    archetypes.insert({new_archetype_id, std::make_unique<Archetype>(new_archetype_id)});
    log_debug("Creating new archetype %b", new_archetype_id);

    // add new archetype to systems that are a subset of it
    for (auto& [_, system] : systems) {
        if ((system->archetype_id & new_archetype_id) == system->archetype_id) {
            system->archetypes.push_back(archetypes.at(new_archetype_id).get());
        }
    }
}

void EntityComponentSystem::update_system_archetype(const char* system_type) {
    std::shared_ptr<System> system = systems.at(system_type);
    ArchetypeID new_archetype_id = system_archetype_map.at(system_type);
    system->archetypes.clear();
    system->archetype_id = new_archetype_id;

    // find all superset archetypes and add to system
    for (auto& [archetype_id, archetype] : archetypes) {
        if ((archetype_id & new_archetype_id) == new_archetype_id) {
            system->archetypes.push_back(archetype.get());
        }
    }
}

