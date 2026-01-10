#include "blocks/chunk_manager.hpp"
#include "utils/logger.hpp"

ChunkManager::ChunkManager(int seed, int chunk_radius) : seed(seed), chunk_radius(chunk_radius) {}

// Given the current player position, determine which chunks to load/unload
void ChunkManager::update(glm::vec3 player_pos) {
    glm::ivec2 chunk_pos = glm::floor(glm::vec2(player_pos.x / CHUNK_LENGTH, player_pos.y / CHUNK_WIDTH));

    for (int i = -chunk_radius - 2; i <= chunk_radius + 2; i++) {
        for (int j = -chunk_radius - 2; j <= chunk_radius + 2; j++) {

            glm::ivec2 current_chunk(chunk_pos.x + i, chunk_pos.y + j);
            int dist = glm::floor(glm::length(glm::vec2(i, j)));

            // add chunk to unload queue if too far away
            if (dist >= chunk_radius + 2) {
                if (chunk_index_map.contains(current_chunk) && !unload_chunk_queue_set.contains(current_chunk)) {
                    unload_chunk_queue.push(current_chunk);
                    unload_chunk_queue_set.insert(current_chunk);
                }
                continue;
            }
            if (dist > chunk_radius) {
                continue;
            }
            // skip loading if chunk already loaded/in load queue
            if (chunk_index_map.contains(current_chunk) || load_chunk_queue_set.contains(current_chunk)) {
                continue;
            }

            load_chunk_queue.push(current_chunk);
            load_chunk_queue_set.insert(current_chunk);
        }
    }

    while (!events.empty()) {
        Event event = std::move(events.front());
        events.pop();

        if (BlockEvent* block_event = std::get_if<BlockEvent>(&event.event)) {
            glm::vec3 block_pos = block_event->pos;
            glm::ivec2 chunk_pos = glm::floor(glm::vec2(block_pos.x / CHUNK_LENGTH, block_pos.y / CHUNK_WIDTH));

            // TODO: add to unloaded chunk queue to update when the chunk gets loaded?
            if (!chunk_index_map.contains(chunk_pos)) {
                log_error("Update accessed unloaded chunk");
                break;
            }

            Chunk& chunk = loaded_chunks[chunk_index_map.at(chunk_pos)];
            switch (block_event->type) {
                case BlockEvent::Type::Break:
                    chunk.delete_block(block_pos);
                    break;
                case BlockEvent::Type::Place:
                    chunk.add_block(block_pos, block_event->block);
                    break;
            }

            if (!update_chunk_queue_set.contains(chunk_pos)) {
                update_chunk_queue.push(chunk_pos);
            }
        }
    }
}

void ChunkManager::load_chunks(int num_chunks, TextureManager& texture_manager) {
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

void ChunkManager::unload_chunks(int num_chunks) {
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

void ChunkManager::update_chunks(int num_chunks, TextureManager& texture_manager) {
    for (int i = 0; i < num_chunks; i++) {
        if (update_chunk_queue.empty()) {
            break;
        }

        glm::ivec2 chunk_pos = update_chunk_queue.front();
        update_chunk_queue.pop();
        update_chunk_queue_set.erase(chunk_pos);

        log_debug("Updating chunk %d, %d", chunk_pos.x, chunk_pos.y);
        Chunk& chunk = loaded_chunks[chunk_index_map.at(chunk_pos)];
        chunk.convert_to_mesh(texture_manager);
    }
}

std::vector<Chunk>& ChunkManager::get_chunks() {
    return loaded_chunks;
}

// Set get_adjacent to true to get the adjacent block on the face of the hit block: used for placing blocks
std::optional<glm::vec3> ChunkManager::cast_ray(glm::vec3 position, glm::vec3 direction, bool get_adjacent) {
    glm::vec3 signs(direction.x > 0.0f ? 1.0f : -1.0f, direction.y > 0.0f ? 1.0f : -1.0f, direction.z > 0.0f ? 1.0f : -1.0f);
    glm::vec3 reciprocals = 1.0f / glm::abs(direction);
    glm::vec3 voxel_pos = glm::floor(position);
    glm::vec3 distances = (signs + 1.0f)/2.0f - (position - voxel_pos) / direction;

    while (glm::distance(position, voxel_pos) < 10.0f) {
        float min_dist = glm::min(distances.x, glm::min(distances.y, distances.z));
        int min_axis = distances.x == min_dist
            ? 0
            : distances.y == min_dist
            ? 1
            : 2;

        voxel_pos[min_axis] += signs[min_axis];
        glm::ivec2 chunk_pos = glm::floor(glm::vec2(voxel_pos.x / CHUNK_LENGTH, voxel_pos.y / CHUNK_WIDTH));
        if (!chunk_index_map.contains(chunk_pos)) {
            log_error("Raycasting accessed unloaded chunk");
            break;
        }

        Chunk& chunk = loaded_chunks[chunk_index_map.at(chunk_pos)];
        Block hit_block = chunk.get_block(chunk.to_chunk_pos(voxel_pos));

        if (hit_block.type != Block::Type::AIR) {
            if (get_adjacent) {
                voxel_pos[min_axis] -= signs[min_axis];
                return voxel_pos;
            }
            return voxel_pos;
        }
        distances[min_axis] += reciprocals[min_axis];
    }
    return std::nullopt;
}

void ChunkManager::add_event(Event event) {
    events.push(std::move(event));
}
