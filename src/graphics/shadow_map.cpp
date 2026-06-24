#include "graphics/shadow_map.hpp"

#include "utils/assert.hpp"
#include "utils/logger.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"

ShadowMap::ShadowMap(int width, int height) : width(width), height(height) {
    // create new framebuffer and texture to store depth information
    glGenFramebuffers(1, &this->FBO);
    glGenTextures(1, &this->texture_id);

    this->texture_unit = Texture::n_textures++;
    glActiveTexture(GL_TEXTURE0 + this->texture_unit);
    glBindTexture(GL_TEXTURE_2D, this->texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float clamp_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clamp_color);

    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->texture_id, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    this->depth_shader = Shader::create("data/shaders/depth.vert", "data/shaders/depth.frag").value_or(0);
    debug_assert(depth_shader != 0, "Error creating depth shader for shadow map");

    this->light_pos = glm::vec3(10.0f, 20.0f, 50.0f);
    this->light_proj = glm::ortho(-40.0f,40.0f, -40.0f, 40.0f, 0.01f, 100.0f);
}

void ShadowMap::shadow_pass(CameraSystem* camera_system, const std::vector<RenderCall>& render_queue) {
    // switch to depth framebuffer and change viewport to shadow map dimensions
    glUseProgram(depth_shader);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, this->width, this->height);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0 + this->texture_unit);

    // TODO: add cascaded shadow maps
    this->light_view = glm::lookAt(camera_system->camera_position() + light_pos,
                                   camera_system->camera_position(),
                                   glm::vec3(0.0f, 0.0f, 1.0f));

    Shader::set_uniform(depth_shader, "view", light_view);
    Shader::set_uniform(depth_shader, "projection", light_proj);

    // change to front face culling to reduce peter-panning effect, then render depth map
    glCullFace(GL_FRONT);
    for (const RenderCall& render_call : render_queue) {
        Shader::set_uniform(depth_shader, "model", render_call.transform);
        glBindVertexArray(render_call.VAO);
        glDrawArrays(GL_TRIANGLES, 0, render_call.n_vertices);
    }
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint ShadowMap::get_unit() const {
    return this->texture_unit;
}

