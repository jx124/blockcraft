#pragma once

#include "ecs/archetype.hpp"
#include "ecs/common.hpp"
#include "events/event.hpp"

#include <unordered_set>
#include <vector>

// Simple container for all the archetypes that match the system archetype or are a superset of it.
class System {
public:
    ~System() = default;

    void delete_unused_archetypes(const std::unordered_set<ArchetypeID>& unused_archetypes);
    void add_event(Event event);

    std::vector<Archetype*> archetypes{};
    ArchetypeID archetype_id{};
    std::queue<Event> events{};
};

