target_include_directories(blockcraft PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(blockcraft_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(blockcraft PUBLIC common.hpp stb_image.h)
target_precompile_headers(blockcraft_server PUBLIC common.hpp stb_image.h)
