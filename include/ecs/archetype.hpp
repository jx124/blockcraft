#pragma once

#include "ecs/common.hpp"
#include "ecs/component.hpp"

#include <unordered_map>
#include <unordered_set>
#include <typeinfo>

// Stores multiple ComponentVectors of an Archetype (a related set of Components) together and
// ensures that the Components across the vectors at the same index belong to the same entity.
class Archetype {
public:
    Archetype(ArchetypeID archetype_id);

    // Move all component data associated with entity to the new archetype.
    void transfer_components(EntityID entity, Archetype* new_archetype);

    template <typename T>
    void add_component(EntityID entity, const T& component) {
        const char* type_name = typeid(T).name();
        if (!components.contains(type_name)) {
            components.insert({type_name, std::make_unique<ComponentVector<T>>()});
        }

        // TODO: check if all components of an entity are added to the same index
        static_cast<ComponentVector<T>*>(borrow_component_vector(type_name))->add_component(entity, component);
        entities.insert(entity);
    }

    template <typename T>
    std::vector<T>& get_component_vector() {
        const char* type_name = typeid(T).name();
        debug_assert(components.contains(type_name), "Component vector does not exist in archetype");

        return static_cast<ComponentVector<T>*>(borrow_component_vector(type_name))->get_component_vector();
    }

    template <typename T>
    T& get_component(EntityID entity) {
        const char* type_name = typeid(T).name();
        debug_assert(components.contains(type_name), "Component does not exist in archetype");

        return static_cast<ComponentVector<T>*>(borrow_component_vector(type_name))->get_component(entity);
    }

    bool has_entity(EntityID entity) const;
    void delete_entity(EntityID entity);
    ArchetypeID get_archetype() const;
    size_t size() const;

    // Clears all components and entities in the archetype. Make sure there are no dangling references to the components.
    void clear();

private:
    // Returns a non-owning pointer to the component vector.
    IComponentVector* borrow_component_vector(const char* type_name);

    ArchetypeID archetype_id;
    std::unordered_map<const char*, std::unique_ptr<IComponentVector>> components{};
    std::unordered_set<EntityID> entities{};
};
