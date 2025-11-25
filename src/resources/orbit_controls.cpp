// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#include "orbit_controls.h"

#include <glm/vec3.hpp>
#include <glm/common.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

namespace {

constexpr float eps = std::numeric_limits<float>::epsilon();
constexpr float pi_over_2 = static_cast<float>(std::numbers::pi / 2);

auto MakeSafe(float theta);
auto SphericalToVec3(float radius, float phi, float theta);

}

OrbitControls::OrbitControls(PerspectiveCamera* camera) : camera_(camera) {
    mouse_event_listener_ = std::make_shared<EventListener>([this](Event* event) {
        auto e = event->As<MouseEvent>();
        if (!e) return;

        curr_pos_ = e->position;
        if (is_first_move_) {
            is_first_move_ = false;
            prev_pos_ = curr_pos_;
            return;
        }

        auto is_pressed = e->type == MouseEvent::Type::ButtonPressed;
        if (e->button == MouseButton::Left) {
            do_orbit_ = is_pressed;
        }

        if (e->type == MouseEvent::Type::Scrolled) {
            scroll_offset_ = e->scroll.y;
            do_zoom_ = scroll_offset_ != 0.0f;
        }
    });

    EventDispatcher::Get().AddEventListener("mouse_event", mouse_event_listener_);
}

auto OrbitControls::Update(float dt) -> void {
    // TODO: implement
}

namespace {

auto SphericalToVec3(float radius, float phi, float theta) {
    const auto c = std::cos(theta);
    return glm::vec3 {
        radius * std::sin(phi) * c,
        radius * std::sin(theta),
        radius * std::cos(phi) * c
    };
}

auto MakeSafe(float theta) {
    const auto limit = pi_over_2 - eps;
    return std::clamp(theta, -limit, limit);
}

}