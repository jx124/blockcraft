#include "graphics/texture.hpp"

#include "utils/logger.hpp"
#include <algorithm>
#include <iterator>

#define STB_IMAGE_IMPLEMENTATION
#include "graphics/stb_image.h"

Texture::Texture(const ImageData& image_data, GLenum target = GL_TEXTURE_2D) : unit(n_textures), target(target) {
    glGenTextures(1, &this->id);
    glActiveTexture(GL_TEXTURE0 + n_textures++);
    glBindTexture(target, this->id);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(target, 0, GL_RGBA8, image_data.width, image_data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data.data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::Texture(const std::vector<ImageData>& image_data, GLenum target = GL_TEXTURE_2D_ARRAY) : unit(n_textures), target(target) {
    std::vector<unsigned char> combined_images;

    bool first = true;
    int width = 0;
    int height = 0;

    for (size_t i = 0; i < image_data.size(); i++) {
        const ImageData& image = image_data[i];
        if (first) {
            width = image.width;
            height = image.height;
        } else {
            debug_assert(width == image.width && height == image.height, "Cannot load images of different dimensions into 2D texture array");
        }
        std::copy(image.data.begin(), image.data.end(), std::back_inserter(combined_images));
    }

    glGenTextures(1, &this->id);
    glActiveTexture(GL_TEXTURE0 + n_textures++);
    glBindTexture(target, this->id);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // allocate enough space for 2d texture array
    glTexStorage3D(target, 3, GL_RGBA8, width, height, image_data.size());

    // send image to mipmap level 0 and let OpenGL generate the rest automatically
    glTexSubImage3D(target, 0, 0, 0, 0, width, height, image_data.size(), GL_RGBA, GL_UNSIGNED_BYTE, combined_images.data());
    glGenerateMipmap(target);
}

std::optional<ImageData> Texture::read_image(const std::filesystem::path& image_path) {
    ImageData image{};
    log_debug("Loading image \"%s\"", image_path.c_str());

    stbi_set_flip_vertically_on_load(true); 
    // force images to be RGBA so we can load them all into an array
    unsigned char* data = stbi_load(image_path.c_str(), &image.width, &image.height, &image.n_channels, 4);

    if (!data) {
        stbi_image_free(data);
        log_error("Error reading image file \"%s\"", image_path);
        return std::nullopt;
    }

    size_t len = static_cast<size_t>(image.width * image.height * 4);
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
