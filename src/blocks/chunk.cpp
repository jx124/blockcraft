#include "blocks/chunk.hpp"

#include "blocks/common.hpp"
#include "glm/gtc/noise.hpp"
#include "logger.hpp"
#include <array>

bool Block::is_transparent(Block block) {
    // TODO: use a map
    return block.type == Block::Type::AIR
        || block.type == Block::Type::GLASS
        || block.type == Block::Type::WATER;
}

VoxelQuad::VoxelQuad(VoxelQuad::Face face, int x, int y, int z, Block::Type block_type, glm::ivec4 ao_state)
    : face(face), chunk_pos(x, y, z), block_type(block_type), ao_state(ao_state) {}

Chunk::Chunk(glm::ivec2 chunk_coords, int seed)
    : chunk_coords(chunk_coords), seed(seed), blocks(BLOCKS_PER_CHUNK, {Block::Type::AIR}) {

    glGenVertexArrays(1, &mesh.VAO);
    glBindVertexArray(mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBufferData(GL_ARRAY_BUFFER, BLOCKS_PER_CHUNK * VERTICES_PER_BLOCK * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);
};

glm::vec3 Chunk::to_world_pos(glm::vec3 chunk_pos) const {
    return { static_cast<float>(chunk_coords.x * CHUNK_LENGTH) + chunk_pos.x,
             static_cast<float>(chunk_coords.y * CHUNK_WIDTH) + chunk_pos.y,
             chunk_pos.z };
}

glm::ivec3 Chunk::to_world_pos(glm::ivec3 chunk_pos) const {
    return { chunk_coords.x * CHUNK_LENGTH + chunk_pos.x,
             chunk_coords.y * CHUNK_WIDTH + chunk_pos.y,
             chunk_pos.z };
}

glm::vec3 Chunk::to_chunk_pos(glm::vec3 world_pos) {
    return glm::mod(world_pos, { CHUNK_LENGTH, CHUNK_WIDTH, CHUNK_HEIGHT });
}

size_t Chunk::to_block_index(glm::vec3 world_pos) const {
    glm::ivec3 chunk_pos = glm::floor(Chunk::to_chunk_pos(world_pos));
    return CHUNK_LENGTH * CHUNK_WIDTH * chunk_pos.z + CHUNK_LENGTH * chunk_pos.y + chunk_pos.x;
}

Block& Chunk::get_block(glm::ivec3 chunk_pos) {
    size_t index = CHUNK_LENGTH * CHUNK_WIDTH * chunk_pos.z + CHUNK_LENGTH * chunk_pos.y + chunk_pos.x;
    return blocks[index];
}

Block Chunk::get_block_copy(glm::ivec3 chunk_pos) const {
    size_t index = CHUNK_LENGTH * CHUNK_WIDTH * chunk_pos.z + CHUNK_LENGTH * chunk_pos.y + chunk_pos.x;
    return blocks[index];
}

std::optional<Block> Chunk::get_block_from_world_pos(glm::vec3 world_pos,
                                                     const std::vector<Chunk>& loaded_chunks,
                                                     const std::unordered_map<glm::ivec2, size_t>& chunk_index_map) {
    glm::ivec2 chunk_pos = glm::floor(glm::vec2(world_pos.x / CHUNK_LENGTH, world_pos.y / CHUNK_WIDTH));

    if (!chunk_index_map.contains(chunk_pos) || world_pos.z < 0 || world_pos.z >= CHUNK_HEIGHT) {
        return std::nullopt;
    }

    const Chunk& chunk = loaded_chunks[chunk_index_map.at(chunk_pos)];
    glm::ivec3 in_chunk_pos = Chunk::to_chunk_pos(world_pos);
    return { chunk.get_block_copy(in_chunk_pos) };
}

void Chunk::generate_blocks_from_seed() {
    for (int i = 0; i < CHUNK_LENGTH; i++) {
        for (int j = 0; j < CHUNK_WIDTH; j++) {
            float x = static_cast<float>(i + chunk_coords.x * CHUNK_LENGTH) / (16 * CHUNK_LENGTH);
            float y = static_cast<float>(j + chunk_coords.y * CHUNK_WIDTH) / (16 * CHUNK_WIDTH);

            // elevation noise in range [0, 0.1]
            float elevation = 0.05f 
                + (glm::perlin(glm::vec3(1 * x, 1 * y, static_cast<float>(seed)))
                + 0.5f * glm::perlin(glm::vec3(2 * x, 2 * y, static_cast<float>(seed)))
                + 0.25f * glm::perlin(glm::vec3(4 * x, 4 * y, static_cast<float>(seed)))) / (2 * 1.75f);

            for (int k = 0; k < CHUNK_HEIGHT; k++) {
                Block& block = get_block({i, j, k});
                int ground_level = static_cast<int>((elevation + 0.3f) * CHUNK_HEIGHT);

                if (k <= ground_level - 5) {
                    block = {Block::Type::STONE};
                } else if (k <= ground_level - 1) {
                    block = {Block::Type::DIRT};
                } else if (k == ground_level) {
                    block = {Block::Type::GRASS};
                }
            }
        }
    }
}

void Chunk::convert_to_mesh(const TextureManager& texture_manager,
                            const std::vector<Chunk>& loaded_chunks,
                            const std::unordered_map<glm::ivec2, size_t>& chunk_index_map) {
    quads.clear();
    mesh.vertices.clear();
    convert_to_quads(loaded_chunks, chunk_index_map);

    for (const VoxelQuad& quad : quads) {
        generate_vertex_data(quad, texture_manager);
    }

    // TODO: separate out into a different function so we can do the conversion on different threads but only
    // send the mesh to the GPU on the main OpenGL thread.
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh.vertices.size() * sizeof(uint32_t), mesh.vertices.data());
}

constexpr std::array<VoxelQuad::Face, 6> faces = {
    VoxelQuad::Face::TOP,
    VoxelQuad::Face::BOTTOM,
    VoxelQuad::Face::LEFT,
    VoxelQuad::Face::RIGHT,
    VoxelQuad::Face::FRONT,
    VoxelQuad::Face::BACK,
};

constexpr std::array<glm::vec3, 6> block_neighbors = {
    glm::vec3{0.0f, 0.0f, 1.0f},  // TOP
    glm::vec3{0.0f, 0.0f, -1.0f}, // BOTTOM
    glm::vec3{-1.0f, 0.0f, 0.0f}, // LEFT
    glm::vec3{1.0f, 0.0f, 0.0f},  // RIGHT
    glm::vec3{0.0f, -1.0f, 0.0f}, // FRONT
    glm::vec3{0.0f, 1.0f, 0.0f},  // BACK
};

// neighboring block positions when looking at face, in a clockwise order from the top left
// used for ambient occlusion calculations
constexpr std::array<std::array<glm::vec3, 8>, 6> face_neighbors = {
    std::array<glm::vec3,8>{ // TOP
        glm::vec3{-1.0f,  1.0f, 1.0f},
        glm::vec3{ 0.0f,  1.0f, 1.0f},
        glm::vec3{ 1.0f,  1.0f, 1.0f},
        glm::vec3{ 1.0f,  0.0f, 1.0f},
        glm::vec3{ 1.0f, -1.0f, 1.0f},
        glm::vec3{ 0.0f, -1.0f, 1.0f},
        glm::vec3{-1.0f, -1.0f, 1.0f},
        glm::vec3{-1.0f,  0.0f, 1.0f},
    },
    std::array<glm::vec3,8>{ // BOTTOM
        glm::vec3{-1.0f, -1.0f, -1.0f},
        glm::vec3{ 0.0f, -1.0f, -1.0f},
        glm::vec3{ 1.0f, -1.0f, -1.0f},
        glm::vec3{ 1.0f,  0.0f, -1.0f},
        glm::vec3{ 1.0f,  1.0f, -1.0f},
        glm::vec3{ 0.0f,  1.0f, -1.0f},
        glm::vec3{-1.0f,  1.0f, -1.0f},
        glm::vec3{-1.0f,  0.0f, -1.0f},
    },
    std::array<glm::vec3,8>{ // LEFT
        glm::vec3{-1.0f,  1.0f,  1.0f},
        glm::vec3{-1.0f,  0.0f,  1.0f},
        glm::vec3{-1.0f, -1.0f,  1.0f},
        glm::vec3{-1.0f, -1.0f,  0.0f},
        glm::vec3{-1.0f, -1.0f, -1.0f},
        glm::vec3{-1.0f,  0.0f, -1.0f},
        glm::vec3{-1.0f,  1.0f, -1.0f},
        glm::vec3{-1.0f,  1.0f,  0.0f},
    },
    std::array<glm::vec3,8>{ // RIGHT
        glm::vec3{1.0f, -1.0f,  1.0f},
        glm::vec3{1.0f,  0.0f,  1.0f},
        glm::vec3{1.0f,  1.0f,  1.0f},
        glm::vec3{1.0f,  1.0f,  0.0f},
        glm::vec3{1.0f,  1.0f, -1.0f},
        glm::vec3{1.0f,  0.0f, -1.0f},
        glm::vec3{1.0f, -1.0f, -1.0f},
        glm::vec3{1.0f, -1.0f,  0.0f},
    },
    std::array<glm::vec3,8>{ // FRONT
        glm::vec3{-1.0f, -1.0f,  1.0f},
        glm::vec3{ 0.0f, -1.0f,  1.0f},
        glm::vec3{ 1.0f, -1.0f,  1.0f},
        glm::vec3{ 1.0f, -1.0f,  0.0f},
        glm::vec3{ 1.0f, -1.0f, -1.0f},
        glm::vec3{ 0.0f, -1.0f, -1.0f},
        glm::vec3{-1.0f, -1.0f, -1.0f},
        glm::vec3{-1.0f, -1.0f,  0.0f},
    },
    std::array<glm::vec3,8>{ // BACK
        glm::vec3{ 1.0f, 1.0f,  1.0f},
        glm::vec3{ 0.0f, 1.0f,  1.0f},
        glm::vec3{-1.0f, 1.0f,  1.0f},
        glm::vec3{-1.0f, 1.0f,  0.0f},
        glm::vec3{-1.0f, 1.0f, -1.0f},
        glm::vec3{ 0.0f, 1.0f, -1.0f},
        glm::vec3{ 1.0f, 1.0f, -1.0f},
        glm::vec3{ 1.0f, 1.0f,  0.0f},
    },
};

// in anti_clockwise order starting from bottom left
std::array<std::array<int, 3>, 4> corner_neighbors = {
    std::array<int, 3>{7, 6, 5},
    std::array<int, 3>{5, 4, 3},
    std::array<int, 3>{3, 2, 1},
    std::array<int, 3>{1, 0, 7},
};

void Chunk::convert_to_quads(const std::vector<Chunk>& loaded_chunks,
                             const std::unordered_map<glm::ivec2, size_t>& chunk_index_map) {
    for (int x = 0; x < CHUNK_LENGTH; x++) {
        for (int y = 0; y < CHUNK_WIDTH; y++) {
            for (int z = 0; z < CHUNK_HEIGHT; z++) {
                const Block& block = get_block({x, y, z});

                if (block.type == Block::Type::AIR) {
                    continue;
                }

                glm::vec3 current_pos = glm::vec3(x, y, z);
                for (VoxelQuad::Face face : faces) {
                    glm::vec3 block_neighbor_pos = to_world_pos(current_pos + block_neighbors[face]);
                    std::optional<Block> block_neighbor = get_block_from_world_pos(block_neighbor_pos,
                                                                                   loaded_chunks,
                                                                                   chunk_index_map);

                    // hidden-face culling
                    if (!block_neighbor || Block::is_transparent(*block_neighbor)) {
                        std::array<int, 8> face_neighbor_opaque{};

                        // check number of opaque blocks in neighborhood of face
                        for (int i = 0; i < 8; i++) {
                            glm::vec3 face_neighbor_pos = to_world_pos(current_pos + face_neighbors[face][i]);
                            std::optional<Block> face_neighbor = get_block_from_world_pos(face_neighbor_pos,
                                                                                          loaded_chunks,
                                                                                          chunk_index_map);
                            if (face_neighbor && !Block::is_transparent(*face_neighbor)) {
                                face_neighbor_opaque[i] = 1;
                            }
                        }

                        // calculate ambient occlusion values for quad corners
                        glm::ivec4 ao_state{};
                        for (int i = 0; i < 4; i++) {
                            int side_1_opaque = face_neighbor_opaque[corner_neighbors[i][0]];
                            int side_2_opaque = face_neighbor_opaque[corner_neighbors[i][2]];

                            if (side_1_opaque + side_2_opaque == 2) {
                                ao_state[i] = 0;
                                continue;
                            }

                            int corner_opaque = face_neighbor_opaque[corner_neighbors[i][1]];

                            ao_state[i] = 3 - (side_1_opaque + side_2_opaque + corner_opaque);
                        }

                        quads.emplace_back(face, x, y, z, block.type, ao_state);
                    }
                }
            }
        }
    }
}

constexpr std::array<float, 4> ao_values = { 0.512f, 0.64f, 0.8f, 1.0f };

void Chunk::generate_vertex_data(const VoxelQuad& quad, const TextureManager& texture_manager) {
    int x = quad.chunk_pos.x;
    int y = quad.chunk_pos.y;
    int z = quad.chunk_pos.z;
    int texture_index{};
    glm::ivec4 ao = quad.ao_state;

    // TODO: refactor out vertex offsets
    switch (quad.face) {
        case VoxelQuad::Face::TOP:
            texture_index = texture_manager.get_texture_index(quad.block_type, VoxelQuad::Face::TOP);
            // have split in quad across from the darkest corner
            if (ao_values[ao[0]] + ao_values[ao[2]] > ao_values[ao[1]] + ao_values[ao[3]]) {
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 0, 0, texture_index, VoxelQuad::Face::TOP, ao[0]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 1, 0, texture_index, VoxelQuad::Face::TOP, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 1, 1, texture_index, VoxelQuad::Face::TOP, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 1, 1, texture_index, VoxelQuad::Face::TOP, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 0, 1, texture_index, VoxelQuad::Face::TOP, ao[3]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 0, 0, texture_index, VoxelQuad::Face::TOP, ao[0]}.pack_data());
            } else {
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 0, 0, texture_index, VoxelQuad::Face::TOP, ao[0]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 1, 0, texture_index, VoxelQuad::Face::TOP, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 0, 1, texture_index, VoxelQuad::Face::TOP, ao[3]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 1, 0, texture_index, VoxelQuad::Face::TOP, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 1, 1, texture_index, VoxelQuad::Face::TOP, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 0, 1, texture_index, VoxelQuad::Face::TOP, ao[3]}.pack_data());

            }
            break;
        case VoxelQuad::Face::BOTTOM:
            texture_index = texture_manager.get_texture_index(quad.block_type, VoxelQuad::Face::BOTTOM);
            if (ao_values[ao[0]] + ao_values[ao[2]] > ao_values[ao[1]] + ao_values[ao[3]]) {
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 0, 0, texture_index, VoxelQuad::Face::BOTTOM, ao[0]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 1, 0, texture_index, VoxelQuad::Face::BOTTOM, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 1, 1, texture_index, VoxelQuad::Face::BOTTOM, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 1, 1, texture_index, VoxelQuad::Face::BOTTOM, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 0, 1, texture_index, VoxelQuad::Face::BOTTOM, ao[3]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 0, 0, texture_index, VoxelQuad::Face::BOTTOM, ao[0]}.pack_data());
            } else {
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 0, 0, texture_index, VoxelQuad::Face::BOTTOM, ao[0]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 1, 0, texture_index, VoxelQuad::Face::BOTTOM, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 0, 1, texture_index, VoxelQuad::Face::BOTTOM, ao[3]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 1, 0, texture_index, VoxelQuad::Face::BOTTOM, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 1, 1, texture_index, VoxelQuad::Face::BOTTOM, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 0, 1, texture_index, VoxelQuad::Face::BOTTOM, ao[3]}.pack_data());
            }
            break;
        case VoxelQuad::Face::LEFT:
            texture_index = texture_manager.get_texture_index(quad.block_type, VoxelQuad::Face::LEFT);
            if (ao_values[ao[0]] + ao_values[ao[2]] > ao_values[ao[1]] + ao_values[ao[3]]) {
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 0, 0, texture_index, VoxelQuad::Face::LEFT, ao[0]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 1, 0, texture_index, VoxelQuad::Face::LEFT, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 1, 1, texture_index, VoxelQuad::Face::LEFT, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 1, 1, texture_index, VoxelQuad::Face::LEFT, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 0, 1, texture_index, VoxelQuad::Face::LEFT, ao[3]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 0, 0, texture_index, VoxelQuad::Face::LEFT, ao[0]}.pack_data());
            } else {
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 0, 0, texture_index, VoxelQuad::Face::LEFT, ao[0]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 1, 0, texture_index, VoxelQuad::Face::LEFT, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 0, 1, texture_index, VoxelQuad::Face::LEFT, ao[3]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 1, 0, texture_index, VoxelQuad::Face::LEFT, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 1, 1, texture_index, VoxelQuad::Face::LEFT, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 0, 1, texture_index, VoxelQuad::Face::LEFT, ao[3]}.pack_data());
            }
            break;
        case VoxelQuad::Face::RIGHT:
            texture_index = texture_manager.get_texture_index(quad.block_type, VoxelQuad::Face::RIGHT);
            if (ao_values[ao[0]] + ao_values[ao[2]] > ao_values[ao[1]] + ao_values[ao[3]]) {
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 0, 0, texture_index, VoxelQuad::Face::RIGHT, ao[0]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 1, 0, texture_index, VoxelQuad::Face::RIGHT, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 1, 1, texture_index, VoxelQuad::Face::RIGHT, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 1, 1, texture_index, VoxelQuad::Face::RIGHT, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 0, 1, texture_index, VoxelQuad::Face::RIGHT, ao[3]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 0, 0, texture_index, VoxelQuad::Face::RIGHT, ao[0]}.pack_data());
            } else {
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 0, 0, texture_index, VoxelQuad::Face::RIGHT, ao[0]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 1, 0, texture_index, VoxelQuad::Face::RIGHT, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 0, 1, texture_index, VoxelQuad::Face::RIGHT, ao[3]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 1, 0, texture_index, VoxelQuad::Face::RIGHT, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 1, 1, texture_index, VoxelQuad::Face::RIGHT, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 0, 1, texture_index, VoxelQuad::Face::RIGHT, ao[3]}.pack_data());
            }
            break;
        case VoxelQuad::Face::FRONT:
            texture_index = texture_manager.get_texture_index(quad.block_type, VoxelQuad::Face::FRONT);
            if (ao_values[ao[0]] + ao_values[ao[2]] > ao_values[ao[1]] + ao_values[ao[3]]) {
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 0, 0, texture_index, VoxelQuad::Face::FRONT, ao[0]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 1, 0, texture_index, VoxelQuad::Face::FRONT, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 1, 1, texture_index, VoxelQuad::Face::FRONT, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 1, 1, texture_index, VoxelQuad::Face::FRONT, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 0, 1, texture_index, VoxelQuad::Face::FRONT, ao[3]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 0, 0, texture_index, VoxelQuad::Face::FRONT, ao[0]}.pack_data());
            } else {
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 0, 0, texture_index, VoxelQuad::Face::FRONT, ao[0]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 1, 0, texture_index, VoxelQuad::Face::FRONT, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 0, 1, texture_index, VoxelQuad::Face::FRONT, ao[3]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 1, 0, texture_index, VoxelQuad::Face::FRONT, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 1, 1, texture_index, VoxelQuad::Face::FRONT, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 0, 1, texture_index, VoxelQuad::Face::FRONT, ao[3]}.pack_data());
            }
            break;
        case VoxelQuad::Face::BACK:
            texture_index = texture_manager.get_texture_index(quad.block_type, VoxelQuad::Face::BACK);
            if (ao_values[ao[0]] + ao_values[ao[2]] > ao_values[ao[1]] + ao_values[ao[3]]) {
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 0, 0, texture_index, VoxelQuad::Face::BACK, ao[0]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 1, 0, texture_index, VoxelQuad::Face::BACK, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 1, 1, texture_index, VoxelQuad::Face::BACK, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 1, 1, texture_index, VoxelQuad::Face::BACK, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 0, 1, texture_index, VoxelQuad::Face::BACK, ao[3]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 0, 0, texture_index, VoxelQuad::Face::BACK, ao[0]}.pack_data());
            } else {
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 0, 0, texture_index, VoxelQuad::Face::BACK, ao[0]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 1, 0, texture_index, VoxelQuad::Face::BACK, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 0, 1, texture_index, VoxelQuad::Face::BACK, ao[3]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 1, 0, texture_index, VoxelQuad::Face::BACK, ao[1]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 1, 1, texture_index, VoxelQuad::Face::BACK, ao[2]}.pack_data());
                mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 0, 1, texture_index, VoxelQuad::Face::BACK, ao[3]}.pack_data());
            }
            break;
    }
}

GLuint Chunk::get_VAO() const {
    return mesh.VAO;
}

size_t Chunk::get_num_vertices() const {
    return mesh.vertices.size();
}

glm::ivec2 Chunk::get_chunk_coords() const {
    return chunk_coords;
}

void Chunk::add_block(glm::vec3 world_pos, Block block) {
    size_t index = to_block_index(world_pos);
    blocks[index] = block;
    log_debug("Adding block %d", blocks[index].type);
}

void Chunk::delete_block(glm::vec3 world_pos) {
    size_t index = to_block_index(world_pos);
    log_debug("Deleting block %d", blocks[index].type);
    blocks[index] = { Block::Type::AIR };
}

void Chunk::clear() {
    glBindVertexArray(0);
    glDeleteBuffers(1, &mesh.VBO);
    glDeleteVertexArrays(1, &mesh.VAO);
}
