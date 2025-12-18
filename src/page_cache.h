// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#pragma once

#include <list>
#include <optional>
#include <unordered_map>
#include <vector>

#include <glm/vec2.hpp>

#include "types.h"

struct ResidencyDecision {
    std::optional<PageSlot> slot;
    std::optional<PageRequest> evicted;
};

class PageCache {
public:
    PageCache(const glm::ivec2& pages, unsigned int min_pinned_lod_idx);

    auto Commit(const PageRequest& request, const PageSlot& slot) -> void;

    auto Cancel(const PageSlot& slot) -> void;

    auto Touch(const PageRequest& request) -> void;

    [[nodiscard]] auto Acquire(const PageRequest& request) -> ResidencyDecision;

private:
    std::vector<PageSlot> free_slots_ {};

    std::list<PageRequest> lru_list_ {};

    std::unordered_map<PageRequest, std::list<PageRequest>::iterator> lru_map_ {};
    std::unordered_map<PageRequest, PageSlot> req_to_slot_ {};

    size_t capacity_;

    unsigned int min_pinned_lod_idx_;
};