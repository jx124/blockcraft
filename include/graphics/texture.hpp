#pragma once

#include "graphics/common.hpp"

#include <filesystem>
#include <optional>
#include <vector>

struct ImageData {
    std::vector<unsigned char> data;
    std::filesystem::path path;
    int width;
    int height;
    int n_channels;
};

class Texture {
public:
    Texture(const ImageData& image_data, GLenum target);

    static std::optional<ImageData> read_image(const std::filesystem::path& image_path);
    GLuint get_id() const;
    GLuint get_slot() const;

private:
    GLuint id{};
    GLuint slot;
    GLenum target;

    // Global variable keeping track of which texture slots have been used.
    // Increment after every call to glActiveTexture.
    static inline GLuint n_textures = 0;
};
