cmake_minimum_required(VERSION 4.3)
project(utilitylib CXX)

set(TARGET_NAME ${PROJECT_NAME})

add_library(${TARGET_NAME}
    src/resource_path.cpp
)

add_library(sim::utilitylib ALIAS ${TARGET_NAME})

target_include_directories(${TARGET_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${TARGET_NAME}
    SDL3::SDL3
    spdlog::spdlog
)
