#include "blocks/chunk_manager.hpp"

#include "utils/logger.hpp"

#include <cstddef>
#include <cstring>
#include <vector>

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
            // skip loading if chunk already loaded/in request queue
            if (chunk_index_map.contains(current_chunk) || request_chunk_queue_set.contains(current_chunk)) {
                continue;
            }

            request_chunk_queue.push(current_chunk);
            request_chunk_queue_set.insert(current_chunk);
        }
    }

    while (!events.empty()) {
        Event event = std::move(events.front());
        events.pop();

        if (BlockEvent* block_event = std::get_if<BlockEvent>(&event.event)) {
            glm::vec3 block_pos = block_event->pos;
            glm::ivec2 chunk_pos = glm::floor(glm::vec2(block_pos.x / CHUNK_LENGTH, block_pos.y / CHUNK_WIDTH));

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

            // remesh corner/neighboring chunks if blocks modified on chunk boundary
            glm::ivec3 in_chunk_pos = glm::floor(Chunk::to_chunk_pos(block_pos));
            if (in_chunk_pos.x == 0 && in_chunk_pos.y == 0) {
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(0, -1));
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(-1, -1));
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(-1, 0));
            } else if (in_chunk_pos.x == 0 && in_chunk_pos.y == CHUNK_WIDTH - 1) {
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(-1, 0));
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(-1, 1));
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(0, 1));
            } else if (in_chunk_pos.x == CHUNK_LENGTH - 1 && in_chunk_pos.y == CHUNK_WIDTH - 1) {
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(0, 1));
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(1, 1));
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(1, 0));
            } else if (in_chunk_pos.x == CHUNK_LENGTH - 1 && in_chunk_pos.y == 0) {
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(1, 0));
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(1, -1));
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(0, -1));
            } else if (in_chunk_pos.x == 0) {
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(-1, 0));
            } else if (in_chunk_pos.y == CHUNK_WIDTH - 1) {
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(0, 1));
            } else if (in_chunk_pos.x == CHUNK_LENGTH - 1) {
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(1, 0));
            } else if (in_chunk_pos.y == 0) {
                mesh_chunk_queue.push(chunk_pos + glm::ivec2(0, -1));
            }
            mesh_chunk_queue.push(chunk_pos);
        }
    }
}

// Send all chunk requests to the server
void ChunkManager::request_all_chunks(ClientInterface& client) {
    while (!request_chunk_queue.empty()) {
        glm::ivec2 chunk_pos = request_chunk_queue.front();
        request_chunk_queue.pop();

        ChunkRequest chunk_request{ chunk_pos, seed };
        client.send(chunk_request.serialize());
    }
};

// Deserialize chunk data received from the server and queue chunk data to be loaded
void ChunkManager::receive_chunk_data(Packet packet) {
    ChunkData chunk_data = ChunkData::deserialize(packet);
    received_chunk_queue.push(std::move(chunk_data));
}

// Generates num_chunks Chunk objects and load their blocks
void ChunkManager::load_chunks(int num_chunks) {
    for (int i = 0; i < num_chunks; i++) {
        if (received_chunk_queue.empty()) {
            break;
        }

        ChunkData chunk_data = received_chunk_queue.front();
        received_chunk_queue.pop();
        glm::ivec2 chunk_pos = chunk_data.chunk_coords;

        // generate Chunk object from ChunkData
        Chunk current_chunk(chunk_data.chunk_coords, chunk_data.seed);
        current_chunk.set_blocks(std::move(chunk_data.blocks));

        // add to loaded chunks list and finally remove from request_chunk_queue_set since we have processed the chuck
        size_t current_index = loaded_chunks.size();
        loaded_chunks.push_back(std::move(current_chunk));
        chunk_index_map.insert({chunk_pos, current_index});
        request_chunk_queue_set.erase(chunk_pos);

        mesh_chunk_queue.push(chunk_pos + glm::ivec2(-1, 0));
        mesh_chunk_queue.push(chunk_pos + glm::ivec2(1, 0));
        mesh_chunk_queue.push(chunk_pos + glm::ivec2(0, -1));
        mesh_chunk_queue.push(chunk_pos + glm::ivec2(0, 1));
        mesh_chunk_queue.push(chunk_pos);

        // allocate GPU buffers first since we may attempt to draw these chunks before they are meshed and sent to the GPU.
        // thus, we will only draw nothing and not a non-existent VAO
        chunk_gpu_handler.allocate_chunk(chunk_pos);
    }
}

// Generates Chunk objects for all chunks in queue and loads their blocks
void ChunkManager::load_all_chunks() {
    while (!received_chunk_queue.empty()) {
        ChunkData chunk_data = received_chunk_queue.front();
        received_chunk_queue.pop();
        glm::ivec2 chunk_pos = chunk_data.chunk_coords;

        // generate Chunk object from ChunkData
        Chunk current_chunk(chunk_data.chunk_coords, chunk_data.seed);
        current_chunk.set_blocks(std::move(chunk_data.blocks));

        // add to loaded chunks list and finally remove from request_chunk_queue_set since we have processed the chuck
        size_t current_index = loaded_chunks.size();
        loaded_chunks.push_back(std::move(current_chunk));
        chunk_index_map.insert({chunk_pos, current_index});
        request_chunk_queue_set.erase(chunk_pos);

        mesh_chunk_queue.push(chunk_pos + glm::ivec2(-1, 0));
        mesh_chunk_queue.push(chunk_pos + glm::ivec2(1, 0));
        mesh_chunk_queue.push(chunk_pos + glm::ivec2(0, -1));
        mesh_chunk_queue.push(chunk_pos + glm::ivec2(0, 1));
        mesh_chunk_queue.push(chunk_pos);

        // allocate GPU buffers first since we may attempt to draw these chunks before they are meshed and sent to the GPU.
        // thus, we will only draw nothing and not a non-existent VAO
        chunk_gpu_handler.allocate_chunk(chunk_pos);
    }
}

// Unloads a Chunk object and deletes it from the GPU
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
            loaded_chunks.pop_back();
            chunk_index_map.erase(chunk_pos);
            continue;
        }

        glm::ivec2 last_chunk = loaded_chunks.back().get_chunk_coords();

        loaded_chunks[chunk_index] = std::move(loaded_chunks.back());
        loaded_chunks.pop_back();

        chunk_index_map.erase(chunk_pos);
        chunk_index_map.at(last_chunk) = chunk_index;

        chunk_gpu_handler.deallocate_chunk(chunk_pos);
    }
}

// Creates the mesh of num_chunks Chunks from their loaded blocks, then sends the meshes to the GPU
void ChunkManager::mesh_chunks(int num_chunks, TextureManager& texture_manager) {
    for (int i = 0; i < num_chunks; i++) {
        if (mesh_chunk_queue.empty()) {
            break;
        }

        glm::ivec2 chunk_pos = mesh_chunk_queue.front();
        mesh_chunk_queue.pop();

        if (!chunk_index_map.contains(chunk_pos)) {
            continue;
        }

        Chunk& chunk = loaded_chunks[chunk_index_map.at(chunk_pos)];
        chunk.convert_to_mesh(texture_manager, loaded_chunks, chunk_index_map);

        chunk_gpu_handler.send_mesh_to_gpu(chunk_pos, chunk.get_vertices());
    }
}

// Creates the mesh of all Chunks in the queue from their loaded blocks, then sends the meshes to the GPU
void ChunkManager::mesh_all_chunks(TextureManager& texture_manager) {
    while (!mesh_chunk_queue.empty()) {
        glm::ivec2 chunk_pos = mesh_chunk_queue.front();
        mesh_chunk_queue.pop();

        if (!chunk_index_map.contains(chunk_pos)) {
            continue;
        }

        Chunk& chunk = loaded_chunks[chunk_index_map.at(chunk_pos)];
        chunk.convert_to_mesh(texture_manager, loaded_chunks, chunk_index_map);

        chunk_gpu_handler.send_mesh_to_gpu(chunk_pos, chunk.get_vertices());
    }
}

std::vector<Chunk>& ChunkManager::get_chunks() {
    return loaded_chunks;
}

GLuint ChunkManager::get_chunk_VAO(glm::ivec2 chunk_coords) const {
    return chunk_gpu_handler.get_chunk_VAO(chunk_coords);
}

// Based on the Amanatides & Woo ray marching algorithm http://www.cse.yorku.ca/~amana/research/grid.pdf
// Set get_adjacent to true to get the adjacent block on the face of the hit block; useful for placing blocks
std::optional<glm::vec3> ChunkManager::cast_ray(glm::vec3 position, glm::vec3 direction, bool get_adjacent) {
    glm::vec3 step = glm::sign(direction);
    glm::vec3 t_delta = 1.0f / glm::abs(direction);
    glm::vec3 voxel_pos = glm::floor(position);
    glm::vec3 next_boundary = voxel_pos + (step + 1.0f) / 2.0f;
    glm::vec3 t_max = (next_boundary - position) / direction;

    int min_axis = t_max.x < t_max.y
        ? (t_max.x < t_max.z ? 0 : 2)
        : (t_max.y < t_max.z ? 1 : 2);
    float t = 0.0f;

    while (t < 10.0f) {
        glm::ivec2 chunk_pos = glm::floor(glm::vec2(voxel_pos.x / CHUNK_LENGTH, voxel_pos.y / CHUNK_WIDTH));
        if (!chunk_index_map.contains(chunk_pos)) {
            log_error("Raycasting accessed unloaded chunk");
            break;
        }

        Chunk& chunk = loaded_chunks[chunk_index_map.at(chunk_pos)];
        Block hit_block = chunk.get_block(chunk.to_chunk_pos(voxel_pos));

        if (hit_block.type != Block::Type::AIR) {
            if (get_adjacent) {
                voxel_pos[min_axis] -= step[min_axis];
                return voxel_pos;
            }
            return voxel_pos;
        }

        min_axis = t_max.x < t_max.y
            ? (t_max.x < t_max.z ? 0 : 2)
            : (t_max.y < t_max.z ? 1 : 2);

        voxel_pos[min_axis] += step[min_axis];
        t = t_max[min_axis];
        t_max[min_axis] += t_delta[min_axis];
    }

    return std::nullopt;
}

void ChunkManager::add_event(Event event) {
    events.push(std::move(event));
}
