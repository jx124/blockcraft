#pragma once

#include "graphics/common.hpp"
#include "systems/camera.hpp"
#include <vector>

constexpr int CASCADE_LEVELS = 4;

class ShadowMap {
public:
    ShadowMap(int width, int height, CameraSystem* camera_system);
    ShadowMap(const ShadowMap&) = delete;
    ShadowMap(ShadowMap&&) noexcept = default;
    ShadowMap& operator=(const ShadowMap&) = delete;
    ShadowMap& operator=(ShadowMap&&) noexcept = default;

    void shadow_pass(const std::vector<RenderCall>& render_queue);
    GLuint get_unit() const;

    int width{};
    int height{};
    CameraSystem* camera_system{};

    std::vector<float> cascade_split_planes{};
    glm::vec3 light_pos{};
private:
    std::vector<glm::vec4> get_frustum_corners_world_space(const glm::mat4& proj, const glm::mat4& view) const;
    glm::mat4 get_light_matrix(float near_plane, float far_plane) const;
    std::vector<glm::mat4> get_light_matrices() const;

    GLuint FBO{};
    GLuint texture_id{};
    GLuint texture_unit{};
    GLuint depth_shader{};
    GLuint matrices_UBO{};
};
