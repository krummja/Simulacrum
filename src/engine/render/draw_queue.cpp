#include "draw_queue.hpp"

namespace engine::render {

    DrawQueue::DrawQueue() {}

    DrawQueue::~DrawQueue() {}

    bool DrawQueue::init() {
        if (!callback_) {
            return false;
        }
        return true;
    }

    void DrawQueue::registerCallback(std::function<void(MeshGPU&)> func) {
        callback_ = std::move(func);
    }

    void DrawQueue::addMesh(MeshGPU* mesh) {
        meshes_.push(mesh);
    }

    void DrawQueue::consume() {
        MeshGPU* next = nullptr;

        while (!meshes_.empty()) {
            next = meshes_.front();
            meshes_.pop();
            callback_(*next);
        }
    }

}
