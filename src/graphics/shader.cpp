#include "graphics/shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <optional>

std::optional<GLuint> Shader::create(const std::string& vertex_path, const std::string& fragment_path) {
    std::optional<std::string> vertex_source = Shader::parse_file(vertex_path);
    std::optional<std::string> fragment_source = Shader::parse_file(fragment_path);

    if (!vertex_source || !fragment_source) {
        return std::nullopt;
    }

    std::optional<GLuint> vertex_shader = Shader::compile(*vertex_source, Shader::Type::Vertex);
    std::optional<GLuint> fragment_shader = Shader::compile(*fragment_source, Shader::Type::Fragment);

    if (!vertex_shader || !fragment_shader) {
        return std::nullopt;
    }

    GLuint program_id = glCreateProgram();
    bool success = Shader::link(program_id, *vertex_shader, *fragment_shader);

    glDeleteShader(*vertex_shader);
    glDeleteShader(*fragment_shader);

    if (success) {
        return { program_id };
    } else {
        return std::nullopt;
    }
}

std::optional<std::string> Shader::parse_file(const std::string& file_path) {
    std::ifstream file{ file_path };

    if (!file.is_open()) {
        std::cerr << "[Shader] Error opening file \"" << file_path << '"' << std::endl;
        return std::nullopt;
    }

    std::stringstream file_stream;

    file_stream << file.rdbuf();
    file.close();

    if (file_stream.bad()) {
        std::cerr << "[Shader] Error reading \"" << file_path << '"' << std::endl;
        return std::nullopt;
    }
    
    return { file_stream.str() };
}

std::optional<GLuint> Shader::compile(const std::string& source, Shader::Type type) {
    GLuint shader = glCreateShader(type);
    const char* source_c_str = source.c_str();
    glShaderSource(shader, 1, &source_c_str, nullptr);
    glCompileShader(shader);

    int success;
    char log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    const char* shader_type_name;
    switch (type) {
        case Shader::Type::Vertex:
            shader_type_name = "vertex";
            break;
        case Shader::Type::Fragment:
            shader_type_name = "fragment";
            break;
        case Shader::Type::Geometry:
            shader_type_name = "geometry";
            break;
    }

    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, log);
        std::cerr << "[Shader] Error compiling " << shader_type_name << " shader:\n\t" << log << std::flush;
        return std::nullopt;
    }

    return { shader };
}

bool Shader::link(GLuint program_id, GLuint vertex_shader, GLuint fragment_shader) {
    glAttachShader(program_id, vertex_shader);
    glAttachShader(program_id, fragment_shader);
    glLinkProgram(program_id);

    int success;
    char log[512];
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    
    if (!success) {
        glGetProgramInfoLog(program_id, 512, NULL, log);
        std::cerr << "[Shader] Error linking shader:\n\t" << log << std::flush;
        return false;
    }

    return true;
}
