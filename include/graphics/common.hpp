#pragma once

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

struct RenderCall {
    glm::mat4 transform;
    GLuint VAO;
    GLuint shader_id;
    GLuint texture_unit;
    size_t n_vertices;
};
