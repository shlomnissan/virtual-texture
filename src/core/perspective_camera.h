// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#pragma once

#include <glm/mat4x4.hpp>

class PerspectiveCamera {
public:
    glm::mat4 transform {1.0f};
    glm::mat4 projection {1.0f};

    PerspectiveCamera(
        float fov,
        float aspect,
        float near,
        float far
    );

    [[nodiscard]] auto View() const -> glm::mat4 {
        return glm::inverse(transform);
    }
};