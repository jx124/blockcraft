#pragma once

#include "graphics/common.hpp"

#include <filesystem>
#include <optional>
#include <string>

class Shader {
public:
    enum Type : GLuint {
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
    };

    static std::optional<GLuint> create(const std::filesystem::path& vertex_path, const std::filesystem::path& fragment_path);

    static void set_uniform(GLuint program_id, const std::string& name, bool value);
    static void set_uniform(GLuint program_id, const std::string& name, int value);
    static void set_uniform(GLuint program_id, const std::string& name, float value);
    static void set_uniform(GLuint program_id, const std::string& name, const glm::vec2& value);
    static void set_uniform(GLuint program_id, const std::string& name, const glm::vec3& value);
    static void set_uniform(GLuint program_id, const std::string& name, const glm::vec4& value);
    static void set_uniform(GLuint program_id, const std::string& name, const glm::mat2& value);
    static void set_uniform(GLuint program_id, const std::string& name, const glm::mat3& value);
    static void set_uniform(GLuint program_id, const std::string& name, const glm::mat4& value);

private:
    static std::optional<std::string> parse_file(const std::filesystem::path& file_path);
    static std::optional<GLuint> compile(const std::string& source, Shader::Type type);
    static bool link(GLuint program_id, GLuint vertex_shader, GLuint fragment_shader);
};
