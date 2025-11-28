// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#pragma once

#include <algorithm>
#include <format>
#include <memory>
#include <random>
#include <vector>
#include <set>
#include <mutex>

#include "core/texture2d.h"
#include "loaders/image_loader.h"

#include "page_table.h"

constexpr int image_h = 4096;
constexpr int image_w = 4096;
constexpr int page_h = 1024;
constexpr int page_w = 1024;
constexpr int pages_x = 4;
constexpr int pages_y = 4;

struct PageRequest {
    uint32_t lod;
    int x;
    int y;

    auto operator<=>(const PageRequest&) const = default;
};

struct PendingUpload {
    PageRequest request;
    int alloc_x;
    int alloc_y;
    std::shared_ptr<Image> image;
};

struct PageManager {
    PageTable page_table {pages_x, pages_y};

    Texture2D atlas {};

    std::vector<int> alloc_arr = std::vector<int>(pages_x * pages_y);
    std::vector<PendingUpload> upload_queue {};

    std::shared_ptr<ImageLoader> loader {ImageLoader::Create()};

    std::set<PageRequest> processing {};

    size_t alloc_idx = 0;

    PageManager() {
        for (size_t i = 0u; i < alloc_arr.size(); ++i) {
            alloc_arr[i] = static_cast<int>(i);
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::ranges::shuffle(alloc_arr, gen);

        atlas.InitTexture(
            image_w, image_h,
            GL_RGBA, GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }

    auto IngestFeedback(const std::vector<uint32_t>& feedback) {
        std::set<PageRequest> requests {};

        for (auto packed : feedback) {
            if (packed != 0xFFFFFFFFu) {
                requests.emplace(
                    packed & 0x1Fu,
                    static_cast<int>((packed >> 5) & 0x3Fu),
                    static_cast<int>((packed >> 11) & 0x3Fu)
                );
            }
        }

        for (auto request : requests) {
            if (
                !page_table.IsResident(request.x, request.y) &&
                processing.find(request) == processing.end()
            ) {
                processing.emplace(request);
                RequestPage(request);
                alloc_idx += 1;
            }
        }
    }

    auto FlushUploadQueue() -> void {
        while (!upload_queue.empty()) {
            auto e = upload_queue.back();
            upload_queue.pop_back();

            atlas.Update(
                page_w * e.alloc_x,
                page_h * e.alloc_y,
                page_w,
                page_h,
                e.image->Data()
            );

            auto entry = uint32_t {0x1 | ((e.alloc_x & 0xFFu) << 1) | ((e.alloc_y & 0xFFu) << 9)};
            page_table.Write(e.request.x, e.request.y, entry);
            processing.erase(e.request);
        }
    }

    auto RequestPage(const PageRequest& request) -> void {
        auto alloc_x = alloc_arr[alloc_idx] % pages_x;
        auto alloc_y = alloc_arr[alloc_idx] / pages_x;
        auto path = std::format("assets/pages/{}_{}_{}.png", request.lod, request.x, request.y);
        loader->LoadAsync(path, [this, request, alloc_x, alloc_y](auto result) {
            if (result) {
                upload_queue.emplace_back(
                    request,
                    alloc_x,
                    alloc_y,
                    std::move(result.value())
                );
            } else {
                std::println("{}", result.error());
            }

        });
    }
};