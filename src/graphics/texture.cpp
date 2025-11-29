#include "graphics/texture.hpp"

#include "utils/logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "graphics/stb_image.h"

Texture::Texture(const ImageData& image_data, GLenum target) : unit(n_textures), target(target) {
    glGenTextures(1, &this->id);
    glActiveTexture(GL_TEXTURE0 + n_textures++);
    glBindTexture(target, this->id);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    constexpr GLenum formats[4] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
    GLenum format = formats[image_data.n_channels - 1];

    glTexImage2D(target, 0, format, image_data.width, image_data.height, 0, format, GL_UNSIGNED_BYTE, image_data.data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
}

std::optional<ImageData> Texture::read_image(const std::filesystem::path& image_path) {
    ImageData image{};
    unsigned char* data = stbi_load(image_path.c_str(), &image.width, &image.height, &image.n_channels, 0);

    if (!data) {
        stbi_image_free(data);
        log_error("Error reading image file \"%s\"", image_path);
        return std::nullopt;
    }

    size_t len = static_cast<size_t>(image.width * image.height * image.n_channels);
    image.data = std::vector<unsigned char>(data, data + len);
    stbi_image_free(data);
    return { image };
}

GLuint Texture::get_id() const {
    return id;
}

GLuint Texture::get_unit() const {
    return unit;
}
