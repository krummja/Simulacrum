#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <queue>
#include "gpu_renderer.hpp"
#include "renderer_types.hpp"

namespace engine::render {

    class DrawQueue final {
        friend class GPURenderer;

    public:
        DrawQueue();
        ~DrawQueue();

        // Copy operations
        DrawQueue(const DrawQueue&) = delete;
        DrawQueue& operator=(const DrawQueue&) = delete;

        // Move operations
        DrawQueue(DrawQueue&&) = delete;
        DrawQueue& operator=(DrawQueue&&) = delete;

        bool init();

    private:
        std::queue<MeshGPU*> meshes_;
        std::function<void(MeshGPU&)> callback_;

        void registerCallback(std::function<void(MeshGPU&)> func);
        void addMesh(MeshGPU* mesh);
        void consume();
    };

} // namespace engine::render
