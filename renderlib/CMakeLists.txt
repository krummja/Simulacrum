cmake_minimum_required(VERSION 4.3)
project(renderlib CXX)

set(TARGET_NAME ${PROJECT_NAME})

add_library(${TARGET_NAME}
    src/gpu_buffer.cpp
    src/gpu_device.cpp
    src/gpu_pipeline.cpp
    src/gpu_renderer.cpp
    src/gpu_sampler.cpp
    src/gpu_shader_manager.cpp
    src/gpu_texture.cpp
    src/gpu_transfer_buffer.cpp
    src/gpu_vertex_pool.cpp
    src/sprite_batch.cpp
)

add_library(sim::renderlib ALIAS ${TARGET_NAME})

target_include_directories(${TARGET_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${TARGET_NAME}
    SDL3::SDL3
    spdlog::spdlog
)
