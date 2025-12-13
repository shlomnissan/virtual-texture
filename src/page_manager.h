// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#pragma once

#include <format>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include <glm/vec2.hpp>

#include "core/texture2d.h"
#include "loaders/image_loader.h"

#include "page_tables.h"
#include "page_cache.h"
#include "types.h"

constexpr auto atlas_size = glm::vec2(4096.0f, 4096.0f);
constexpr auto page_size = glm::vec2(1024.0f, 1024.0f);
constexpr auto pages = glm::ivec2(atlas_size / page_size);

struct PendingUpload {
    PageRequest request;
    PageSlot page_slot;
    std::shared_ptr<Image> image;
};

struct PageManager {
    PageCache page_cache {pages.x, pages.y};

    PageTables page_table;

    Texture2D atlas {};

    std::vector<PendingUpload> upload_queue {};
    std::vector<PendingUpload> failure_queue {};

    std::mutex upload_mutex {};

    std::shared_ptr<ImageLoader> loader {ImageLoader::Create()};

    std::set<PageRequest> processing {};

    size_t alloc_idx = 0;

    PageManager(const glm::ivec2& virtual_size) : page_table(virtual_size, page_size) {
        atlas.InitTexture({
            .width = static_cast<int>(atlas_size.x),
            .height = static_cast<int>(atlas_size.y),
            .levels = 0,
            .internal_format = GL_RGBA,
            .format = GL_RGBA,
            .type = GL_UNSIGNED_BYTE,
            .min_filter = GL_LINEAR,
            .data = nullptr
        });
    }

    auto IngestFeedback(const std::vector<uint32_t>& feedback) {
        std::set<PageRequest> requests {};

        for (auto packed : feedback) {
            if (packed != 0xFFFFFFFFu) {
                requests.emplace(
                    packed & 0x1Fu,
                    static_cast<int>((packed >> 5) & 0xFFu),
                    static_cast<int>((packed >> 13) & 0xFFu)
                );
            }
        }

        for (auto request : requests) {
            page_cache.Touch(request);

            if (
                !page_table.IsResident(request.lod, request.x, request.y) &&
                processing.find(request) == processing.end()
            ) {
                processing.emplace(request);
                RequestPage(request);
            }
        }
    }

    auto FlushUploadQueue() -> void {
        std::vector<PendingUpload> uploads;
        std::vector<PendingUpload> failures;

        {
            auto lock = std::lock_guard(upload_mutex);
            uploads.swap(upload_queue);
            failures.swap(failure_queue);
        }

        for (const auto& f : failures) {
            page_cache.Cancel(f.page_slot);
            processing.erase(f.request);
        }

        for (const auto& u : uploads) {
            atlas.Update(
                page_size.x * u.page_slot.x,
                page_size.y * u.page_slot.y,
                page_size.x,
                page_size.y,
                u.image->Data()
            );

            auto entry = uint32_t {
                0x1 | ((u.page_slot.x & 0xFFu) << 1) | ((u.page_slot.y & 0xFFu) << 9)
            };

            page_table.Write(u.request, entry);
            page_cache.Commit(u.request, u.page_slot);
            processing.erase(u.request);
        }
    }

    auto RequestPage(const PageRequest& request) -> void {
        auto alloc_result = page_cache.Acquire(request);
        if (!alloc_result.slot) {
            std::println(std::cerr, "Unable to allocate physical page");
            processing.erase(request);
            return;
        }

        if (alloc_result.evicted) {
            page_table.Write(alloc_result.evicted.value(), 0u);
        }

        auto slot = alloc_result.slot.value();
        auto path = std::format("assets/pages/{}_{}_{}.png", request.lod, request.x, request.y);

        loader->LoadAsync(path, [this, request, slot](auto loader_result) {
            auto lock = std::lock_guard(upload_mutex);
            if (loader_result) {
                upload_queue.emplace_back(
                    request,
                    slot,
                    std::move(loader_result.value())
                );
            } else {
                std::println("{}", loader_result.error());
                failure_queue.emplace_back(
                    request,
                    slot,
                    nullptr
                );
            }

        });
    }
};