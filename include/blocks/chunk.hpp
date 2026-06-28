#pragma once

#include "blocks/common.hpp"
#include "graphics/common.hpp"
#include "graphics/texture_manager.hpp"

#include <optional>
#include <unordered_map>
#include <vector>

struct Vertex {
    int x{};                 // 5 bits
    int y{};                 // 5 bits
    int z{};                 // 8 bits
    int u{};                 // 1 bit
    int v{};                 // 1 bit
    int texture_index{};     // 7 bits
    int face{};              // 3 bits
    int ambient_occlusion{}; // 2 bits

    uint32_t pack_data() {
        uint32_t data{};
        data |= ((x & 0b11111) << 27);
        data |= ((y & 0b11111) << 22);
        data |= ((z & 0b111111111) << 14);
        data |= ((u & 0b1) << 13);
        data |= ((v & 0b1) << 12);
        data |= ((texture_index & 0b1111111) << 5);
        data |= ((face & 0b111) << 2);
        data |= ((ambient_occlusion & 0b11) << 0);
        return data;
    }
};

struct Mesh {
    std::vector<uint32_t> vertices{};
    GLuint VAO{};
    GLuint VBO{};
    GLuint shader_id{};
    GLuint texture_id{};
};

constexpr int CHUNK_LENGTH = 16;
constexpr int CHUNK_WIDTH = 16;
constexpr int CHUNK_HEIGHT = 256;
constexpr int BLOCKS_PER_CHUNK = CHUNK_LENGTH * CHUNK_WIDTH * CHUNK_HEIGHT;
constexpr int VERTICES_PER_BLOCK = 6 * 6;

class Chunk {
public:
    Chunk(glm::ivec2 chunk_coords, int seed);

    Chunk(const Chunk& other) = delete;
    Chunk& operator=(const Chunk& other) = delete;
    Chunk(Chunk&& other) noexcept = default;
    Chunk& operator=(Chunk&& other) noexcept = default;

    glm::vec3 to_world_pos(glm::vec3 chunk_pos) const;
    glm::ivec3 to_world_pos(glm::ivec3 chunk_pos) const;
    static glm::vec3 to_chunk_pos(glm::vec3 world_pos);

    // TODO: change ordering of storage
    size_t to_block_index(glm::vec3 world_pos) const;
    Block& get_block(glm::ivec3 chunk_pos);
    Block get_block_copy(glm::ivec3 chunk_pos) const;

    static std::optional<Block> get_block_from_world_pos(glm::vec3 world_pos,
                                                         const std::vector<Chunk>& loaded_chunks,
                                                         const std::unordered_map<glm::ivec2, size_t>& chunk_index_map);

    void generate_blocks_from_seed();

    // pass in chunk map so we can access neighboring chunks for mesh generation and lighting calculations
    void convert_to_mesh(const TextureManager& texture_manager,
                         const std::vector<Chunk>& loaded_chunks,
                         const std::unordered_map<glm::ivec2, size_t>& chunk_index_map);

    GLuint get_VAO() const;
    size_t get_num_vertices() const;
    glm::ivec2 get_chunk_coords() const;

    void add_block(glm::vec3 world_pos, Block block);
    void delete_block(glm::vec3 world_pos);

    // Delete the vertex array and buffer objects from the GPU.
    void clear();

private:
    // TODO: check neighboring blocks for face culling and set block type depending on face
    void convert_to_quads(const std::vector<Chunk>& loaded_chunks,
                          const std::unordered_map<glm::ivec2, size_t>& chunk_index_map);
    void generate_vertex_data(const VoxelQuad& quad, const TextureManager& texture_manager);

    glm::ivec2 chunk_coords;
    int seed;
    std::vector<Block> blocks{};
    std::vector<VoxelQuad> quads{};
    Mesh mesh{};
};
