#pragma once

struct Block {
    enum Type : int {
        AIR = 0,
        GRASS,
        STONE,
        DIRT,
        GLASS,
        WATER,
        SIZE
    } type{};

    static bool is_transparent(Block block);
};

// Represents a face of a voxel, always has size 1x1.
struct VoxelQuad {
    enum Face : int {
        TOP = 0,
        BOTTOM,
        LEFT,
        RIGHT,
        FRONT,
        BACK,
    } face{};
    glm::ivec3 chunk_pos{};
    Block::Type block_type{};

    VoxelQuad(VoxelQuad::Face face, int x, int y, int z, Block::Type block_type);
};

