target_include_directories(blockcraft PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(blockcraft_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(blockcraft PUBLIC
    archetype.hpp
    common.hpp
    component.hpp
    ecs.hpp
    entity.hpp
    system.hpp
)

target_precompile_headers(blockcraft_server PUBLIC
    archetype.hpp
    common.hpp
    component.hpp
    ecs.hpp
    entity.hpp
    system.hpp
)

