cmake_minimum_required(VERSION 3.26.3)
project(Simulacrum VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/prebuilt)

set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# Compiler settings (C++ standard, compilation options, etc.)
include(cmake/CompilerSettings.cmake)
include(cmake/RuntimePath.cmake)
# include(cmake/Dependencies.cmake)
include(cmake/BuildHelpers.cmake)
 include(cmake/ProjectInfo.cmake)

# setup_project_dependencies()

set(SOURCES
        src/main.cpp
)

if(WIN32)
    list(APPEND SOURCES resources.rc)
endif()

add_executable(${TARGET} ${SOURCES})

setup_compiler_options(${TARGET})

setup_asset_copy(${TARGET})

print_project_info(${TARGET})
