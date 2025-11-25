// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#include "orbit_controls.h"

#include <glm/vec3.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace {

constexpr float pi_over_2 {static_cast<float>(std::numbers::pi / 2)};
constexpr float pi_times_2 {static_cast<float>(std::numbers::pi * 2)};
constexpr float orbit_speed {3.5f};
constexpr float zoom_speed {5.0f};
constexpr float pitch_limit = pi_over_2 - 0.001f;

auto SphericalToVec3(float radius, float phi, float theta) -> glm::vec3;

}

OrbitControls::OrbitControls(PerspectiveCamera* camera) : camera_(camera) {
    using enum MouseEvent::Type;

    mouse_event_listener_ = std::make_shared<EventListener>([this](Event* event) {
        auto e = event->As<MouseEvent>();
        if (!e) return;

        if (e->type == Moved) {
            curr_pos_ = e->position;
            if (is_first_move_) {
                is_first_move_ = false;
                prev_pos_ = curr_pos_;
                return;
            }
        }

        if (e->type == ButtonPressed || e->type == ButtonReleased) {
            if (e->button == MouseButton::Left) {
                if (e->type == ButtonPressed) {
                    do_orbit_ = true;
                    is_first_move_ = true;
                } else {
                    do_orbit_ = false;
                }
            }
        }

        if (e->type == Scrolled) {
            scroll_offset_ = e->scroll.y;
        }
    });

    EventDispatcher::Get().AddEventListener("mouse_event", mouse_event_listener_);
}

auto OrbitControls::Update(float dt) -> void {
    if (camera_ == nullptr) return;

    auto offset = curr_pos_ - prev_pos_;

    if (do_orbit_) {
        phi -= offset.x * orbit_speed * dt;
        if (phi > pi_times_2 || phi < -pi_times_2) {
            phi = std::fmod(phi, pi_times_2);
        }

        theta += offset.y * orbit_speed * dt;
        theta = std::clamp(theta, -pitch_limit, pitch_limit);
    }

    if (scroll_offset_ != 0.0f) {
        radius -= scroll_offset_ * zoom_speed * dt;
        radius = std::max(0.1f, radius);
        scroll_offset_ = 0.0f;
    }

    prev_pos_ = curr_pos_;

    camera_->transform = glm::lookAt(
        SphericalToVec3(radius, phi, theta),
        glm::vec3 {0.0f},
        glm::vec3 {0.0f, 1.0f, 0.0f}
    );
}

OrbitControls::~OrbitControls() {
    EventDispatcher::Get().RemoveEventListener("mouse_event", mouse_event_listener_);
}

namespace {

auto SphericalToVec3(float radius, float phi, float theta) -> glm::vec3 {
    const auto c = std::cos(theta);
    return glm::vec3 {
        radius * std::sin(phi) * c,
        radius * std::sin(theta),
        radius * std::cos(phi) * c
    };
}

}