#pragma once

#include "blocks/chunk.hpp"
#include "blocks/chunk_gpu_handler.hpp"
#include "blocks/common.hpp"
#include "utils/logger.hpp"

#include <queue>
#include <unordered_map>
#include <unordered_set>

class ChunkManager {
public:
    ChunkManager(int seed, int chunk_radius);

    // Given the current player position, determine which chunks to load/unload
    void update(glm::vec3 player_pos);
    void load_chunks(int num_chunks);
    void load_all_chunks();
    void unload_chunks(int num_chunks);
    void mesh_chunks(int num_chunks, TextureManager& texture_manager);
    void mesh_all_chunks(TextureManager& texture_manager);
    std::vector<Chunk>& get_chunks();
    GLuint get_chunk_VAO(glm::ivec2 chunk_coords) const;

    // Set get_adjacent to true to get the adjacent block on the face of the hit block: used for placing blocks
    std::optional<glm::vec3> cast_ray(glm::vec3 position, glm::vec3 direction, bool get_adjacent);

    void add_event(Event event);

private:
    int seed;
    int chunk_radius;

    std::vector<Chunk> loaded_chunks{};
    std::unordered_map<glm::ivec2, size_t> chunk_index_map{};

    std::unordered_set<glm::ivec2> load_chunk_queue_set{};
    std::unordered_set<glm::ivec2> unload_chunk_queue_set{};

    std::queue<glm::ivec2> load_chunk_queue{};
    std::queue<glm::ivec2> unload_chunk_queue{};
    std::queue<glm::ivec2> mesh_chunk_queue{};

    std::queue<Event> events{};

    ChunkGPUHandler chunk_gpu_handler{};
};
