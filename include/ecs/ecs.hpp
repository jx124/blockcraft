#pragma once

#include "ecs/archetype.hpp"
#include "ecs/common.hpp"
#include "ecs/entity.hpp"
#include "ecs/system.hpp"

#include "utils/logger.hpp"

#include <unordered_map>

// Keeps track of Entities and their Components, and Systems and their Components.
// Every unique combination of Components an Entity has, is an Archetype.
class EntityComponentSystem {
public:
    EntityComponentSystem() = default;

    std::optional<EntityID> create_entity();
    void delete_entity(EntityID entity);
    void clear_unused_archetypes();

    template <typename T>
    void add_component_to_entity(EntityID entity, const T& component) {
        const char* type_name = typeid(T).name();

        if (!type_component_map.contains(type_name)) {
            register_component_type(type_name);
        }

        ComponentID component_id = type_component_map.at(type_name);
        add_entity_component_to_archetype<T>(entity, component_id, component);
    }

    template <typename T>
        requires std::is_base_of_v<System, T>
    std::shared_ptr<T> register_system() {

        const char* type_name = typeid(T).name();
        debug_assert(!systems.contains(type_name), "System already registered");

        log_debug("Registering system %s", type_name);

        std::shared_ptr<T> system = std::make_shared<T>();
        systems.insert({type_name, system});
        system_archetype_map.insert({type_name, 0});
        return system;
    }

    template <typename System, typename... Components>
    void add_components_to_system() {
        const char* system_type_name = typeid(System).name();
        if (!system_archetype_map.contains(system_type_name)) {
            register_system<System>();
        }

        // use fold expression to OR together all individual component bits
        ArchetypeID archetype_id = (... | get_component_archetype_bit<Components>());

        log_debug("Adding archetype %b to system %s", archetype_id, system_type_name);

        system_archetype_map.at(system_type_name) |= archetype_id;
        update_system_archetype(system_type_name);
    }

private:
    void register_component_type(const char* type_name);
    void create_new_archetype(ArchetypeID new_archetype_id);

    // Clear all old archetypes from the system and repopulate with all superset archetypes fitting the system.
    void update_system_archetype(const char* system_type);

    template <typename T>
    ArchetypeID get_component_archetype_bit() {
        const char* component_type_name = typeid(T).name();

        log_debug("Getting component archetype bit for %s", component_type_name);

        if (!type_component_map.contains(component_type_name)) {
            register_component_type(component_type_name);
        }

        ComponentID component_id = type_component_map.at(component_type_name);
        return 1 << component_id;
    }

    template <typename T>
    void add_entity_component_to_archetype(EntityID entity, ComponentID component_id, const T& component) {
        ArchetypeID old_archetype_id = 0;

        if (entity_archetype_map.contains(entity)) {
             old_archetype_id = entity_archetype_map.at(entity);
        }

        // update entity archetype map
        debug_assert(old_archetype_id.test(component_id) == false, "Component already added to archetype");
        ArchetypeID new_archetype_id = old_archetype_id;
        new_archetype_id.set(component_id, true);
        entity_archetype_map.insert_or_assign(entity, new_archetype_id);

        // create a new archetype if it doesn't exist
        if (!archetypes.contains(new_archetype_id)) {
            create_new_archetype(new_archetype_id);
        }

        Archetype* new_archetype = archetypes.at(new_archetype_id).get();

        // copy the old data over to the new archetype if old data exists
        if (old_archetype_id != 0) {
            Archetype* old_archetype = archetypes.at(old_archetype_id).get();
            old_archetype->transfer_components(entity, new_archetype);
        }

        // add new component to new archetype
        new_archetype->add_component(entity, component);
    }

    EntityManager entity_manager{};
    ComponentID current_component_id = 0;
    std::unordered_map<EntityID, ArchetypeID> entity_archetype_map{};
    std::unordered_map<ArchetypeID, std::unique_ptr<Archetype>> archetypes{};
    std::unordered_map<const char*, ComponentID> type_component_map{};
    std::unordered_map<const char*, ArchetypeID> system_archetype_map{};

    // TODO: check if we need shared_ptrs here, since the event system might not need to own the systems
    std::unordered_map<const char*, std::shared_ptr<System>> systems{};
};

