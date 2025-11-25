// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#pragma once

#include "core/perspective_camera.h"
#include "core/event_dispatcher.h"

#include <memory>

class OrbitControls {
public:
    float radius = 0.0f;
    float phi = 0.0f;
    float theta = 0.0f;

    explicit OrbitControls(PerspectiveCamera* camera);

    auto Update(float dt) -> void;

private:
    PerspectiveCamera* camera_;

    std::shared_ptr<EventListener> mouse_event_listener_;

    bool do_orbit {false};
    bool do_pan {false};
    bool do_zoom {false};
};
