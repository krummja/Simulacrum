# ==============================================
# Project profile
# ==============================================
cmake_minimum_required(VERSION 3.26.3)
project(Simulacrum VERSION 0.1.0 LANGUAGES CXX)

# ==============================================
# Basic configuration
# ==============================================

# Specify the search path (used by find_package to find libraries at compile time)
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/prebuilt)

# Specify the target name
set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# Dependency library default link type:
#   ON = dynamic link (.dll.so/.dylib)
#   OFF = static link (.lib/.a)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# ==============================================
# Configuration modules
# ==============================================

# Compiler settings (C++ standard, compilation options, etc.)
include(cmake/CompilerSettings.cmake)

# Runtime path configuration (RPATH)
include(cmake/RuntimePath.cmake)

# Dependency management
include(cmake/Dependencies.cmake)

# Build helper functions
include(cmake/BuildHelpers.cmake)

# Project information printing
include(cmake/ProjectInfo.cmake)

# ==============================================
# Setting up project dependencies
# ==============================================

# Call the dependency configuration function (defined in Dependencies.cmake)
setup_project_dependencies()

# ==============================================
# Project source files
# ==============================================

set(SOURCES
        src/main.cpp
        src/engine/core/game_app.cpp
        src/engine/core/config.cpp
        src/engine/core/context.cpp
        src/engine/input/input_manager.cpp
        src/engine/scene/scene_manager.cpp
        src/engine/scene/scene.cpp
)

if(WIN32)
    list(APPEND SOURCES resources.rc)
endif()

# ==============================================
# Executable file configuration
# ==============================================

# Create an executable file
add_executable(${TARGET} ${SOURCES})

# Link all dependent libraries
target_link_libraries(${TARGET}
        SDL3::SDL3
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)

# ==============================================
# Application configuration
# ==============================================

# Set compilation options (defined in CompilerSettings.cmake)
setup_compiler_options(${TARGET})

# Configuration resource file copy (defined in BuildHelpers.cmake)
setup_asset_copy(${TARGET})

# Configure Windows DLL replication (defined in BuildHelpers.cmake)
setup_windows_dll_copy(${TARGET})

# ==============================================
# Emscripten specific configuration
# ==============================================

# TODO

# ==============================================
# Print configuration information
# ==============================================

# Print project configuration completion information (defined in ProjectInfo.cmake)
print_project_info(${TARGET})
