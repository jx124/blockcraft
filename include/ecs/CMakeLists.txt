target_include_directories(blockcraft PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(blockcraft PUBLIC
    archetype.hpp
    common.hpp
    component.hpp
    ecs.hpp
    entity.hpp
    system.hpp
)

