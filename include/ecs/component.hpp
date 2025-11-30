#pragma once

#include "ecs/common.hpp"

#include "utils/assert.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

class IComponentVector {
public:
    virtual ~IComponentVector() = default;
    virtual void remove_component(EntityID entity) = 0;
    virtual void transfer_component(EntityID entity, IComponentVector* recipient) = 0;
    virtual std::unique_ptr<IComponentVector> make_empty_clone() const = 0;
};

// A tightly packed vector containing generic Components (structs storing data).
template <typename T>
class ComponentVector : public IComponentVector {
public:
    ComponentVector() = default;

    void add_component(EntityID entity, const T& component) {
        debug_assert(!entity_to_index_map.contains(entity), "Trying to add component to entity more than once");

        size_t last_index = components.size();
        components.push_back(component);
        entity_to_index_map.insert({entity, last_index});
        index_to_entity_map.insert({last_index, entity});
    }


    virtual void remove_component(EntityID entity) override {
        debug_assert(entity_to_index_map.contains(entity), "Trying to remove component from entity that does not have it");

        size_t curr_index = entity_to_index_map.at(entity);

        // only component in vector, skip swapping to end to delete
        // no need to check if curr_index = 0 since .at() will throw if entity not found anyway
        if (components.size() == 1) {
            components.pop_back();
            entity_to_index_map.erase(entity);
            index_to_entity_map.erase(0);
            return;
        }

        size_t last_index = components.size() - 1;
        EntityID last_entity = index_to_entity_map[last_index];

        components[curr_index] = std::move(components.back());
        components.pop_back();

        entity_to_index_map[last_entity] = curr_index;
        index_to_entity_map[curr_index] = last_entity;

        entity_to_index_map.erase(entity);
        index_to_entity_map.erase(last_index);
    }

    // Transfer entity to another ComponentVector of the same type.
    virtual void transfer_component(EntityID entity, IComponentVector* recipient) override {
        debug_assert(entity_to_index_map.contains(entity), "Entity for transfer does not exist in component");

        ComponentVector<T>* recipient_ptr = static_cast<ComponentVector<T>*>(recipient);
        recipient_ptr->add_component(entity, get_component(entity));
        remove_component(entity);
    }

    T& get_component(EntityID entity) {
        debug_assert(entity_to_index_map.contains(entity), "Trying to get component from entity that does not have it");

        size_t index = entity_to_index_map.at(entity);
        return components[index];
    }

    std::vector<T>& get_component_vector() {
        return components;
    }

    virtual std::unique_ptr<IComponentVector> make_empty_clone() const override {
        return std::make_unique<ComponentVector<T>>();
    }

private:
    std::vector<T> components{};
    std::unordered_map<EntityID, size_t> entity_to_index_map{};
    std::unordered_map<size_t, EntityID> index_to_entity_map{};
};
