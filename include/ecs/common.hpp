#pragma once

#include <bitset>
#include <cstdint>

using EntityID = uint32_t;
constexpr size_t MAX_ENTITY_COUNT = 1024;

using ComponentID = uint32_t;
constexpr size_t MAX_COMPONENT_TYPES = 8 * sizeof(ComponentID);

using ArchetypeID = std::bitset<MAX_COMPONENT_TYPES>;
