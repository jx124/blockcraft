#include "graphics/texture_manager.hpp"
#include "utils/logger.hpp"

TextureManager::TextureManager() {
    block_texture_map.resize(Block::Type::SIZE);
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
    log_debug("Texture not yet initialized in TextureManager. Did you call generate_texture_array()?");
    return 0;
}
