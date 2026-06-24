#include "blocks/chunk.hpp"

#include "blocks/common.hpp"
#include "glm/gtc/noise.hpp"
#include "logger.hpp"

bool Block::is_transparent(Block block) {
    // TODO: use a map
    return block.type == Block::Type::AIR
        || block.type == Block::Type::GLASS
        || block.type == Block::Type::WATER;
}

VoxelQuad::VoxelQuad(VoxelQuad::Face face, int x, int y, int z, Block::Type block_type)
    : face(face), chunk_pos(x, y, z), block_type(block_type) {}

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

void Chunk::convert_to_mesh(const TextureManager& texture_manager) {
    quads.clear();
    mesh.vertices.clear();
    convert_to_quads();

    for (const VoxelQuad& quad : quads) {
        generate_vertex_data(quad, texture_manager);
    }
                
    // TODO: separate out into a different function so we can do the conversion on different threads but only
    // send the mesh to the GPU on the main OpenGL thread.
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh.vertices.size() * sizeof(uint32_t), mesh.vertices.data());
}

void Chunk::convert_to_quads() {
    for (int x = 0; x < CHUNK_LENGTH; x++) {
        for (int y = 0; y < CHUNK_WIDTH; y++) {
            for (int z = 0; z < CHUNK_HEIGHT; z++) {
                const Block& block = get_block({x, y, z});

                if (block.type == Block::Type::AIR) {
                    continue;
                }

                // only add quads whose neighbors are transparent
                if ((z + 1 < CHUNK_HEIGHT && Block::is_transparent(get_block({x, y, z + 1}))) || z + 1 == CHUNK_HEIGHT) {
                    quads.emplace_back(VoxelQuad::Face::TOP, x, y, z, block.type);
                }

                if ((z - 1 >= 0 && Block::is_transparent(get_block({x, y, z - 1}))) || z == 0) {
                    quads.emplace_back(VoxelQuad::Face::BOTTOM, x, y, z, block.type);
                }

                if ((y + 1 < CHUNK_WIDTH && Block::is_transparent(get_block({x, y + 1, z}))) || y + 1 == CHUNK_WIDTH) {
                    quads.emplace_back(VoxelQuad::Face::BACK, x, y, z, block.type);
                }

                if ((y - 1 >= 0 && Block::is_transparent(get_block({x, y - 1, z}))) || y == 0) {
                    quads.emplace_back(VoxelQuad::Face::FRONT, x, y, z, block.type);
                }

                if ((x + 1 < CHUNK_LENGTH && Block::is_transparent(get_block({x + 1, y, z}))) || x + 1 == CHUNK_LENGTH) {
                    quads.emplace_back(VoxelQuad::Face::RIGHT, x, y, z, block.type);
                }

                if ((x - 1 >= 0 && Block::is_transparent(get_block({x - 1, y, z}))) || x == 0) {
                    quads.emplace_back(VoxelQuad::Face::LEFT, x, y, z, block.type);
                }
            }
        }
    }
}

void Chunk::generate_vertex_data(const VoxelQuad& quad, const TextureManager& texture_manager) {
    int x = quad.chunk_pos.x;
    int y = quad.chunk_pos.y;
    int z = quad.chunk_pos.z;
    int texture_index{};

    switch (quad.face) {
        case VoxelQuad::Face::TOP:
            texture_index = texture_manager.get_texture_index(quad.block_type, VoxelQuad::Face::TOP);
            mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 0, 0, texture_index, VoxelQuad::Face::TOP}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 1, 0, texture_index, VoxelQuad::Face::TOP}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 1, 1, texture_index, VoxelQuad::Face::TOP}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 1, 1, texture_index, VoxelQuad::Face::TOP}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 0, 1, texture_index, VoxelQuad::Face::TOP}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 0, 0, texture_index, VoxelQuad::Face::TOP}.pack_data());
            break;
        case VoxelQuad::Face::BOTTOM:
            texture_index = texture_manager.get_texture_index(quad.block_type, VoxelQuad::Face::BOTTOM);
            mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 0, 0, texture_index, VoxelQuad::Face::BOTTOM}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 1, 0, texture_index, VoxelQuad::Face::BOTTOM}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 1, 1, texture_index, VoxelQuad::Face::BOTTOM}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 1, 1, texture_index, VoxelQuad::Face::BOTTOM}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 0, 1, texture_index, VoxelQuad::Face::BOTTOM}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 0, 0, texture_index, VoxelQuad::Face::BOTTOM}.pack_data());
            break;
        case VoxelQuad::Face::LEFT:
            texture_index = texture_manager.get_texture_index(quad.block_type, VoxelQuad::Face::LEFT);
            mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 1, 0, texture_index, VoxelQuad::Face::LEFT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 1, 1, texture_index, VoxelQuad::Face::LEFT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 0, 1, texture_index, VoxelQuad::Face::LEFT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 0, 1, texture_index, VoxelQuad::Face::LEFT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 0, 0, texture_index, VoxelQuad::Face::LEFT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 1, 0, texture_index, VoxelQuad::Face::LEFT}.pack_data());
            break;
        case VoxelQuad::Face::RIGHT:
            texture_index = texture_manager.get_texture_index(quad.block_type, VoxelQuad::Face::RIGHT);
            mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 1, 0, texture_index, VoxelQuad::Face::RIGHT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 1, 1, texture_index, VoxelQuad::Face::RIGHT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 0, 1, texture_index, VoxelQuad::Face::RIGHT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 0, 1, texture_index, VoxelQuad::Face::RIGHT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 0, 0, texture_index, VoxelQuad::Face::RIGHT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 1, 0, texture_index, VoxelQuad::Face::RIGHT}.pack_data());
            break;
        case VoxelQuad::Face::FRONT:
            texture_index = texture_manager.get_texture_index(quad.block_type, VoxelQuad::Face::FRONT);
            mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 0, 1, texture_index, VoxelQuad::Face::FRONT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 0, 1, 1, texture_index, VoxelQuad::Face::FRONT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 1, 0, texture_index, VoxelQuad::Face::FRONT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 0, z + 1, 1, 0, texture_index, VoxelQuad::Face::FRONT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 1, 0, 0, texture_index, VoxelQuad::Face::FRONT}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 0, z + 0, 0, 1, texture_index, VoxelQuad::Face::FRONT}.pack_data());
            break;
        case VoxelQuad::Face::BACK:
            texture_index = texture_manager.get_texture_index(quad.block_type, VoxelQuad::Face::BACK);
            mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 0, 1, texture_index, VoxelQuad::Face::BACK}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 0, 1, 1, texture_index, VoxelQuad::Face::BACK}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 1, 0, texture_index, VoxelQuad::Face::BACK}.pack_data());
            mesh.vertices.push_back(Vertex{x + 1, y + 1, z + 1, 1, 0, texture_index, VoxelQuad::Face::BACK}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 1, 0, 0, texture_index, VoxelQuad::Face::BACK}.pack_data());
            mesh.vertices.push_back(Vertex{x + 0, y + 1, z + 0, 0, 1, texture_index, VoxelQuad::Face::BACK}.pack_data());
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
