#pragma once

#include "blocks/common.hpp"
#include "graphics/common.hpp"
#include "graphics/texture_manager.hpp"

#include <vector>

struct Vertex {
    float x{};
    float y{};
    float z{};
    float u{};
    float v{};
    int texture_index{};
    int face{};
};

struct Mesh {
    std::vector<Vertex> vertices{};
    GLuint VAO{};
    GLuint VBO{};
    GLuint shader_id{};
    GLuint texture_id{};
};

constexpr int CHUNK_LENGTH = 16;
constexpr int CHUNK_WIDTH = 16;
constexpr int CHUNK_HEIGHT = 384;
constexpr int BLOCKS_PER_CHUNK = CHUNK_LENGTH * CHUNK_WIDTH * CHUNK_HEIGHT;

class Chunk {
public:
    Chunk(glm::ivec2 chunk_coords, int seed);

    glm::vec3 to_world_pos(glm::vec3 chunk_pos) const;
    glm::vec3 to_chunk_pos(glm::vec3 world_pos) const;

    // TODO: change ordering of storage
    size_t to_block_index(glm::vec3 world_pos) const;
    Block& get_block(glm::ivec3 chunk_pos);

    void generate_blocks_from_seed();
    void convert_to_mesh(const TextureManager& texture_manager);

    GLuint get_VAO() const;
    size_t get_num_vertices() const;

private:
    // TODO: check neighboring blocks for face culling and set block type depending on face
    void convert_to_quads();
    void generate_vertex_data(const VoxelQuad& quad, const TextureManager& texture_manager);
    
    glm::ivec2 chunk_coords;
    int seed;
    std::vector<Block> blocks{};
    std::vector<VoxelQuad> quads{};
    Mesh mesh{};
};
