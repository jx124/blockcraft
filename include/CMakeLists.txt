target_include_directories(blockcraft PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(app)
add_subdirectory(ecs)
add_subdirectory(events)
add_subdirectory(graphics)
add_subdirectory(systems)
add_subdirectory(utils)
