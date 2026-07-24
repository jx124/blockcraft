#include "blocks/chunk_gpu_handler.hpp"

void ChunkGPUHandler::allocate_chunk(glm::ivec2 chunk_coords) {
    if (chunk_buffers_map.contains(chunk_coords)) {
        return;
    }
    Buffers buffers{};

    glGenVertexArrays(1, &buffers.VAO);
    glBindVertexArray(buffers.VAO);
    glGenBuffers(1, &buffers.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.VBO);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBufferData(GL_ARRAY_BUFFER, BLOCKS_PER_CHUNK * VERTICES_PER_BLOCK * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);

    chunk_buffers_map.insert({ chunk_coords, buffers });
}

void ChunkGPUHandler::deallocate_chunk(glm::ivec2 chunk_coords) {
    if (!chunk_buffers_map.contains(chunk_coords)) {
        return;
    }
    Buffers buffers = chunk_buffers_map.at(chunk_coords);

    glBindVertexArray(0);
    glDeleteBuffers(1, &buffers.VBO);
    glDeleteVertexArrays(1, &buffers.VAO);

    chunk_buffers_map.erase(chunk_coords);
}

void ChunkGPUHandler::send_mesh_to_gpu(glm::ivec2 chunk_coords, const std::vector<uint32_t>& vertices) {
    if (!chunk_buffers_map.contains(chunk_coords)) {
        return;
    }
    Buffers buffers = chunk_buffers_map.at(chunk_coords);

    glBindVertexArray(buffers.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(uint32_t), vertices.data());
}

GLuint ChunkGPUHandler::get_chunk_VAO(glm::ivec2 chunk_coords) const {
    if (!chunk_buffers_map.contains(chunk_coords)) {
        log_error("Chunk (%d, %d) is not allocated on the GPU", chunk_coords.x, chunk_coords.y);
        return 0;
    }

    return chunk_buffers_map.at(chunk_coords).VAO;
}
