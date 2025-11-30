#include "ecs/archetype.hpp"

Archetype::Archetype(ArchetypeID archetype_id) : archetype_id(archetype_id) {};

void Archetype::transfer_components(EntityID entity, Archetype* new_archetype) {
    for (auto& [component_type_name, component_vector] : components) {
        if (!new_archetype->components.contains(component_type_name)) {
            new_archetype->components.insert({component_type_name, component_vector->make_empty_clone()});
        }

        IComponentVector* new_component_vector = new_archetype->borrow_component_vector(component_type_name);
        component_vector->transfer_component(entity, new_component_vector);
    }
    entities.erase(entity);
}

bool Archetype::has_entity(EntityID entity) const {
    return entities.contains(entity);
}

void Archetype::delete_entity(EntityID entity) {
    entities.erase(entity);

    for (auto& [_, component] : components) {
        component->remove_component(entity);
    }
}

ArchetypeID Archetype::get_archetype() const {
    return archetype_id;
}

size_t Archetype::size() const {
    return entities.size();
}

void Archetype::clear() {
    components.clear();
    entities.clear();
}

IComponentVector* Archetype::borrow_component_vector(const char* type_name) {
    debug_assert(components.contains(type_name), "Component pointer does not exist in archetype");

    return components.at(type_name).get();
}
