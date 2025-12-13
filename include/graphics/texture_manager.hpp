#pragma once

#include "graphics/texture.hpp"
#include "blocks/common.hpp"

#include <array>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

// index order follows that of VoxelQuad::Face: TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK.
using BlockTexture = std::array<int, 6>;

class TextureManager {
public:
    TextureManager();

    // Set the texture to be used by all faces of a block
    void register_block(const std::filesystem::path& image_path, Block::Type block_type);

    // Set the texture to be used by a specific face of a block
    void register_block_face(const std::filesystem::path& image_path, Block::Type block_type, VoxelQuad::Face block_face);

    void generate_texture_array();
    int get_texture_index(Block::Type block_type, VoxelQuad::Face block_face) const;
    GLuint get_texture_unit() const;

private:
    std::vector<BlockTexture> block_texture_map{};
    std::vector<ImageData> texture_images{};
    std::unordered_map<std::filesystem::path, int> loaded_images{};
    std::unique_ptr<Texture> texture{};
};
