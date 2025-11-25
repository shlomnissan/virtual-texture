// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#include <array>
#include <print>
#include <vector>

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

#include "types.h"

#include "core/framebuffer.h"
#include "core/perspective_camera.h"
#include "core/shaders.h"
#include "core/texture2d.h"
#include "core/window.h"
#include "geometries/plane_geometry.h"
#include "resources/orbit_controls.h"
#include "shaders/headers/feedback_vert.h"
#include "shaders/headers/feedback_frag.h"
#include "shaders/headers/page_vert.h"
#include "shaders/headers/page_frag.h"

#include "page_manager.h"

constexpr auto window_size = glm::vec2(1024.0f, 1024.0f);

auto main() -> int {
    auto window = Window {
        static_cast<int>(window_size.x),
        static_cast<int>(window_size.y),
        "Virtual Texture"
    };

    auto camera = PerspectiveCamera {
        /* fov= */ 60.0f,
        /* aspect= */ window_size.x / window_size.y,
        /* near= */ 0.1f,
        /* far= */ 1000.0f
    };

    auto controls = OrbitControls {&camera};
    controls.radius = 2.0f;

    camera.transform = glm::translate(
        camera.transform,
        {0.0f, 0.0f, 2.0f}
    );

    const auto geometry = PlaneGeometry {{
        .width = 1.0f,
        .height = 1.0f,
        .width_segments = 1,
        .height_segments = 1
    }};

    auto page_shader = Shaders {{
        {ShaderType::kVertexShader, _SHADER_page_vert},
        {ShaderType::kFragmentShader, _SHADER_page_frag}
    }};

    const auto mainPass = [&]() {
        glViewport(0, 0, window.BufferWidth(), window.BufferHeight());
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        page_shader.Use();
        page_shader.SetUniform("u_Projection", camera.projection);
        page_shader.SetUniform("u_ModelView", camera.transform);

        geometry.Draw(page_shader);
    };

    window.Start([&]([[maybe_unused]] const double dt){
        controls.Update(static_cast<float>(dt));
        mainPass();
    });

    return 0;
}