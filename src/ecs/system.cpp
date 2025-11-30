#include "ecs/system.hpp"

#include <algorithm>

void System::delete_unused_archetypes(const std::unordered_set<ArchetypeID>& unused_archetypes) {
    archetypes.erase(
        std::remove_if(
            archetypes.begin(),
            archetypes.end(),
            [&unused_archetypes](Archetype* archetype){
                return unused_archetypes.contains(archetype->get_archetype());
            }
        ),
        archetypes.end()
    );
}
