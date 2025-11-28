#pragma once

#include "graphics/common.hpp"

#include <optional>
#include <string>

class Shader {
public:
    enum Type : GLuint {
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
    };

    // Not using string_view since OS file reading requires null-terminated strings
    static std::optional<GLuint> create(const std::string& vertex_path, const std::string& fragment_path);

private:
    static std::optional<std::string> parse_file(const std::string& file_path);
    static std::optional<GLuint> compile(const std::string& source, Shader::Type type);
    static bool link(GLuint program_id, GLuint vertex_shader, GLuint fragment_shader);
};
