#include "graphics/shader.hpp"

#include "utils/logger.hpp"

#include <fstream>
#include <sstream>
#include <optional>

std::optional<GLuint> Shader::create(const std::filesystem::path& vertex_path, const std::filesystem::path& fragment_path) {
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

    glDetachShader(program_id, *vertex_shader);
    glDetachShader(program_id, *fragment_shader);
    glDeleteShader(*vertex_shader);
    glDeleteShader(*fragment_shader);

    if (success) {
        return { program_id };
    } else {
        return std::nullopt;
    }
}

std::optional<std::string> Shader::parse_file(const std::filesystem::path& file_path) {
    std::ifstream file{ file_path };

    if (!file.is_open()) {
        log_error("Error opening shader file \"%s\"", file_path.c_str());
        return std::nullopt;
    }

    std::stringstream file_stream;

    file_stream << file.rdbuf();
    file.close();

    if (file_stream.bad()) {
        log_error("Error reading shader file \"%s\"", file_path.c_str());
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
        log_error("Error compiling %s shader:\n%s", shader_type_name, log);
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
        log_error("Error linking shader:\n%s", log);
        return false;
    }

    return true;
}

void Shader::set_uniform(GLuint program_id, const std::string& name, bool value) {
    glUniform1i(glGetUniformLocation(program_id, name.c_str()), (int)value);
};

void Shader::set_uniform(GLuint program_id, const std::string& name, int value) {
    glUniform1i(glGetUniformLocation(program_id, name.c_str()), value);
};

void Shader::set_uniform(GLuint program_id, const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(program_id, name.c_str()), value);
};

void Shader::set_uniform(GLuint program_id, const std::string& name, const glm::vec2& value) {
    glUniform2fv(glGetUniformLocation(program_id, name.c_str()), 1, glm::value_ptr(value));
};

void Shader::set_uniform(GLuint program_id, const std::string& name, const glm::vec3& value) {
    glUniform3fv(glGetUniformLocation(program_id, name.c_str()), 1, glm::value_ptr(value));
};

void Shader::set_uniform(GLuint program_id, const std::string& name, const glm::vec4& value) {
    glUniform4fv(glGetUniformLocation(program_id, name.c_str()), 1, glm::value_ptr(value));
};

void Shader::set_uniform(GLuint program_id, const std::string& name, const glm::mat2& value) {
    glUniformMatrix2fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
};

void Shader::set_uniform(GLuint program_id, const std::string& name, const glm::mat3& value) {
    glUniformMatrix3fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
};

void Shader::set_uniform(GLuint program_id, const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
};
