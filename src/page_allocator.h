// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#pragma once

#include <algorithm>
#include <expected>
#include <random>
#include <vector>

#include <glm/vec2.hpp>

struct PageAlloc {
    int x;
    int y;
};

struct PageAllocator {
    std::vector<int> alloc_arr;

    glm::ivec2 pages;

    size_t alloc_idx {0};

    PageAllocator(const glm::ivec2& pages) :
        pages(pages),
        alloc_arr(pages.x * pages.y)
    {
        for (size_t i = 0u; i < alloc_arr.size(); ++i) {
            alloc_arr[i] = static_cast<int>(i);
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::ranges::shuffle(alloc_arr, gen);
    }

    auto Alloc() -> std::expected<PageAlloc, std::string> {
        if (alloc_idx >= alloc_arr.size()) {
            return std::unexpected("Out of memory!");
        }

        auto out = PageAlloc {
            alloc_arr[alloc_idx] % pages.x,
            alloc_arr[alloc_idx] / pages.x
        };

        alloc_idx++;

        return out;
    }
};