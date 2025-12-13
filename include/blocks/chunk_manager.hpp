#pragma once

#include "blocks/chunk.hpp"
#include "utils/logger.hpp"

#include <queue>
#include <unordered_map>
#include <unordered_set>

class ChunkManager {
public:
    ChunkManager(int seed, int chunk_radius) : seed(seed), chunk_radius(chunk_radius) {}

    // Given the current player position, determine which chunks to load/unload
    void update(glm::vec3 player_pos) {
        glm::ivec2 chunk_pos = glm::floor(glm::vec2(player_pos.x / CHUNK_LENGTH, player_pos.y / CHUNK_WIDTH));

        for (int i = -chunk_radius - 2; i <= chunk_radius + 2; i++) {
            for (int j = -chunk_radius - 2; j <= chunk_radius + 2; j++) {
                glm::ivec2 current_chunk(chunk_pos.x + i, chunk_pos.y + j);

                if (glm::abs(i) + glm::abs(j) >= chunk_radius + 2) {
                    if (chunk_index_map.contains(current_chunk) && !unload_chunk_queue_set.contains(current_chunk)) {
                        unload_chunk_queue.push(current_chunk);
                        unload_chunk_queue_set.insert(current_chunk);
                    }
                    continue;
                }
                if (glm::abs(i) + glm::abs(j) > chunk_radius + 1 || glm::abs(i) > chunk_radius || glm::abs(j) > chunk_radius) {
                    continue;
                }
                if (chunk_index_map.contains(current_chunk) || load_chunk_queue_set.contains(current_chunk)) {
                    continue;
                }
                load_chunk_queue.push(current_chunk);
                load_chunk_queue_set.insert(current_chunk);
            }
        }
    }

    void load_chunks(int num_chunks, TextureManager& texture_manager) {
        for (int i = 0; i < num_chunks; i++) {
            if (load_chunk_queue.empty()) {
                break;
            }

            glm::ivec2 chunk_pos = load_chunk_queue.front();
            load_chunk_queue.pop();
            load_chunk_queue_set.erase(chunk_pos);

            Chunk current_chunk(chunk_pos, seed);
            current_chunk.generate_blocks_from_seed();
            current_chunk.convert_to_mesh(texture_manager);

            size_t current_index = loaded_chunks.size();
            loaded_chunks.push_back(std::move(current_chunk));
            chunk_index_map.insert({chunk_pos, current_index});
        }
    }

    void unload_chunks(int num_chunks) {
        for (int i = 0; i < num_chunks; i++) {
            if (unload_chunk_queue.empty()) {
                break;
            }

            glm::ivec2 chunk_pos = unload_chunk_queue.front();
            unload_chunk_queue.pop();
            unload_chunk_queue_set.erase(chunk_pos);

            debug_assert(chunk_index_map.contains(chunk_pos), "Unloading non-existent chunk");
            size_t chunk_index = chunk_index_map.at(chunk_pos);

            // skip swapping to end to remove if chunk is already at the end: causes double erase and segfaults
            if (chunk_index == loaded_chunks.size() - 1) {
                loaded_chunks[chunk_index].clear();
                loaded_chunks.pop_back();
                chunk_index_map.erase(chunk_pos);
                continue;
            }

            glm::ivec2 last_chunk = loaded_chunks.back().get_chunk_coords();

            loaded_chunks[chunk_index].clear();
            loaded_chunks[chunk_index] = std::move(loaded_chunks.back());
            loaded_chunks.pop_back();

            chunk_index_map.erase(chunk_pos);
            chunk_index_map.at(last_chunk) = chunk_index;
        }
    }

    std::vector<Chunk>& get_chunks() {
        return loaded_chunks;
    }

private:
    int seed;
    int chunk_radius;

    std::vector<Chunk> loaded_chunks{};
    std::unordered_map<glm::ivec2, size_t> chunk_index_map{};

    std::unordered_set<glm::ivec2> load_chunk_queue_set{};
    std::unordered_set<glm::ivec2> unload_chunk_queue_set{};

    std::queue<glm::ivec2> load_chunk_queue{};
    std::queue<glm::ivec2> unload_chunk_queue{};
};
