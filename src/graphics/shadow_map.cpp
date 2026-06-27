#include "graphics/shadow_map.hpp"

#include "graphics/shader.hpp"
#include "graphics/texture.hpp"
#include "utils/assert.hpp"
#include "utils/logger.hpp"

#include <cmath>

ShadowMap::ShadowMap(int width, int height, CameraSystem* camera_system) : width(width), height(height), camera_system(camera_system) {
    // create new framebuffer and texture to store depth information
    glGenFramebuffers(1, &this->FBO);
    glGenTextures(1, &this->texture_id);

    this->texture_unit = Texture::n_textures++;
    glActiveTexture(GL_TEXTURE0 + this->texture_unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, this->texture_id);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, width, height, CASCADE_LEVELS,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float clamp_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, clamp_color);

    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->texture_id, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    this->depth_shader = Shader::create("data/shaders/depth.vert", "data/shaders/depth.frag", "data/shaders/depth.geom").value_or(0);
    debug_assert(depth_shader != 0, "Error creating depth shader for shadow map");

    this->light_pos = glm::normalize(glm::vec3(10.0f, 20.0f, 50.0f));

    // create uniform buffer object to send light matrices and split distances to GPU
    glGenBuffers(1, &matrices_UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matrices_UBO);
    // note: floats/ints have an alignment of 16 bytes, hence the 4 * sizeof(float)
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16 + sizeof(float) * 4 * 16 + sizeof(int) * 4, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matrices_UBO);

    float near = camera_system->near_plane();
    float far = camera_system->far_plane();
    constexpr float mix = 0.95f;

    // exponential splitting to maintain constant perspective error
    for (int i = 0; i < CASCADE_LEVELS; i++) {
        float split = mix * near * std::pow(far / near, float(i + 1) / CASCADE_LEVELS) + (1.0f - mix) * (near + (i + 1) * (far - near) / CASCADE_LEVELS);
        this->cascade_split_planes.push_back(split);
        glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(glm::mat4x4) + i * 4 * sizeof(float), sizeof(float), &split);
    }

    glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(glm::mat4x4) + 4 * 16 * sizeof(float), sizeof(int), &CASCADE_LEVELS);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ShadowMap::shadow_pass(const std::vector<RenderCall>& render_queue) {
    // switch to depth framebuffer and change viewport to shadow map dimensions
    glUseProgram(depth_shader);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, this->width, this->height);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0 + this->texture_unit);

    std::vector<glm::mat4> light_matrices = get_light_matrices();

    // send light matrices to UBO
    glBindBuffer(GL_UNIFORM_BUFFER, matrices_UBO);
    for (size_t i = 0; i < CASCADE_LEVELS; ++i) {
        glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &light_matrices[i]);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    for (const RenderCall& render_call : render_queue) {
        Shader::set_uniform(depth_shader, "model", render_call.transform);
        glBindVertexArray(render_call.VAO);
        glDrawArrays(GL_TRIANGLES, 0, render_call.n_vertices);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint ShadowMap::get_unit() const {
    return this->texture_unit;
}

// converts [-1, 1]^3 NDC cube to frustum corner coordinates in world space
std::vector<glm::vec4> ShadowMap::get_frustum_corners_world_space(const glm::mat4& proj, const glm::mat4& view) const {
    glm::mat4 inverse = glm::inverse(proj * view);
    std::vector<glm::vec4> result{};

    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            for (int z = 0; z < 2; z++) {
                glm::vec4 point = inverse * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                result.push_back(point / point.w);
            }
        }
    }

    return result;
}

glm::mat4 ShadowMap::get_light_matrix(float near_plane, float far_plane) const {
    glm::mat4 split_proj = camera_system->projection(near_plane, far_plane);
    std::vector<glm::vec4> corners = get_frustum_corners_world_space(split_proj, camera_system->view());

    // find center of frustums by averaging
    glm::vec3 center(0.0f);
    for (const auto& v : corners) {
        center += glm::vec3(v);
    }
    center /= 8.0f;

    glm::mat4 light_view = glm::lookAt(center + light_pos, center, glm::vec3(0.0f, 0.0f, 1.0f));

    // find AABB for frustum
    float min_x = std::numeric_limits<float>::max();
    float max_x = std::numeric_limits<float>::lowest();
    float min_y = std::numeric_limits<float>::max();
    float max_y = std::numeric_limits<float>::lowest();
    float min_z = std::numeric_limits<float>::max();
    float max_z = std::numeric_limits<float>::lowest();
    for (const auto& v : corners) {
        const auto corner_light_space = light_view * v;
        min_x = std::min(min_x, corner_light_space.x);
        max_x = std::max(max_x, corner_light_space.x);
        min_y = std::min(min_y, corner_light_space.y);
        max_y = std::max(max_y, corner_light_space.y);
        min_z = std::min(min_z, corner_light_space.z);
        max_z = std::max(max_z, corner_light_space.z);
    }

    // allow geometry beyond frustum to cast shadows
    constexpr float x_mult = 2.0f;
    if (min_x < 0) {
        min_x *= x_mult;
    } else {
        min_x /= x_mult;
    }
    if (max_x < 0) {
        max_x /= x_mult;
    } else {
        max_x *= x_mult;
    }
    constexpr float y_mult = 2.0f;
    if (min_y < 0) {
        min_y *= y_mult;
    } else {
        min_y /= y_mult;
    }
    if (max_y < 0) {
        max_y /= y_mult;
    } else {
        max_y *= y_mult;
    }
    constexpr float z_mult = 5.0f;
    if (min_z < 0) {
        min_z *= z_mult;
    } else {
        min_z /= z_mult;
    }
    if (max_z < 0) {
        max_z /= z_mult;
    } else {
        max_z *= z_mult;
    }

    glm::mat4 light_proj = glm::ortho(min_x, max_x, min_y, max_y, min_z, max_z);
    return light_proj * light_view;
}

std::vector<glm::mat4> ShadowMap::get_light_matrices() const {
    std::vector<glm::mat4> result{};

    for (int i = 0; i < CASCADE_LEVELS; i++) {
        if (i == 0) {
            result.push_back(get_light_matrix(camera_system->near_plane(), cascade_split_planes[i]));
        } else {
            result.push_back(get_light_matrix(cascade_split_planes[i - 1], cascade_split_planes[i]));
        }
    }

    return result;
}
