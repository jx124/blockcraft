#pragma once

#include "graphics/common.hpp"
#include "systems/camera.hpp"
#include <vector>

class ShadowMap {
public:
    ShadowMap(int width, int height);
    ShadowMap(const ShadowMap&) = delete;
    ShadowMap(ShadowMap&&) noexcept = default;
    ShadowMap& operator=(const ShadowMap&) = delete;
    ShadowMap& operator=(ShadowMap&&) noexcept = default;

    void shadow_pass(CameraSystem* camera_system, const std::vector<RenderCall>& render_queue);
    GLuint get_unit() const;

    int width{};
    int height{};
    glm::vec3 light_pos{};
    glm::mat4 light_proj{};
    glm::mat4 light_view{};
private:
    GLuint FBO{};
    GLuint texture_id{};
    GLuint texture_unit{};
    GLuint depth_shader{};
};
