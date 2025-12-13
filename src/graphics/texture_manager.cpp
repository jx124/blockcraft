#include "graphics/texture_manager.hpp"

TextureManager::TextureManager() {
    block_texture_map.resize(Block::Type::SIZE);

    this->register_block_face("data/assets/grass.png", Block::Type::GRASS, VoxelQuad::Face::TOP);
    this->register_block_face("data/assets/grass_side.png", Block::Type::GRASS, VoxelQuad::Face::LEFT);
    this->register_block_face("data/assets/grass_side.png", Block::Type::GRASS, VoxelQuad::Face::RIGHT);
    this->register_block_face("data/assets/grass_side.png", Block::Type::GRASS, VoxelQuad::Face::FRONT);
    this->register_block_face("data/assets/grass_side.png", Block::Type::GRASS, VoxelQuad::Face::BACK);
    this->register_block_face("data/assets/dirt.png", Block::Type::DIRT, VoxelQuad::Face::BOTTOM);
    this->register_block("data/assets/stone.png", Block::Type::STONE);
    this->register_block("data/assets/dirt.png", Block::Type::DIRT);
    this->register_block("data/assets/glass.png", Block::Type::GLASS);
    this->register_block("data/assets/water.png", Block::Type::WATER);
    this->generate_texture_array();
}

// Set the texture to be used by all faces of a block
void TextureManager::register_block(const std::filesystem::path& image_path, Block::Type block_type) {
    if (!loaded_images.contains(image_path)) {
        std::optional<ImageData> image =  Texture::read_image(image_path);
        if (!image) {
            return;
        }

        int curr_index = static_cast<int>(texture_images.size());
        texture_images.push_back(std::move(*image));
        loaded_images.insert({image_path, curr_index});
    }

    int curr_index = loaded_images.at(image_path);
    block_texture_map[block_type] = {
        curr_index,
        curr_index,
        curr_index,
        curr_index,
        curr_index,
        curr_index,
    };
}

// Set the texture to be used by a specific face of a block
void TextureManager::register_block_face(const std::filesystem::path& image_path, Block::Type block_type, VoxelQuad::Face block_face) {
    if (!loaded_images.contains(image_path)) {
        std::optional<ImageData> image =  Texture::read_image(image_path);
        if (!image) {
            return;
        }

        int curr_index = static_cast<int>(texture_images.size());
        texture_images.push_back(std::move(*image));
        loaded_images.insert({image_path, curr_index});
    }

    int curr_index = loaded_images.at(image_path);
    block_texture_map[block_type][block_face] = curr_index;
}

void TextureManager::generate_texture_array() {
    texture = std::make_unique<Texture>(texture_images, GL_TEXTURE_2D_ARRAY);
}

int TextureManager::get_texture_index(Block::Type block_type, VoxelQuad::Face block_face) const {
    return block_texture_map[block_type][block_face];
}

GLuint TextureManager::get_texture_unit() const {
    if (texture) {
        return texture->get_unit();
    }
    return 0;
}
