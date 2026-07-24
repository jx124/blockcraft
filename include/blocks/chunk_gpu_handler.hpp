#pragma once

#include "graphics/common.hpp"
#include "blocks/chunk.hpp"
#include "utils/logger.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

struct Buffers {
    GLuint VAO;
    GLuint VBO;
};

class ChunkGPUHandler {
public:
    void allocate_chunk(glm::ivec2 chunk_coords);
    void deallocate_chunk(glm::ivec2 chunk_coords);
    void send_mesh_to_gpu(glm::ivec2 chunk_coords, const std::vector<uint32_t>& vertices);
    GLuint get_chunk_VAO(glm::ivec2 chunk_coords) const;

private:
    std::unordered_map<glm::ivec2, Buffers> chunk_buffers_map{};
};
