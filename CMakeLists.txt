# ==============================================
# Project profile
# ==============================================

cmake_minimum_required(VERSION 3.5...4.3)

project(Simulacrum VERSION 0.1.0 LANGUAGES C CXX)

# ==============================================
# Basic configuration
# ==============================================

# Specify the search path (used by find_package to find libraries at compile time)
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/prebuilt)

# Specify the target name
set(EXECUTABLE_TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

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

# Shader compilation
include(cmake/CompileShaders.cmake)

# ==============================================
# Setting up project dependencies
# ==============================================

# Call the dependency configuration function (defined in Dependencies.cmake)
setup_project_dependencies()

# ==============================================
# Executable file configuration
# ==============================================

file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create an executable file
add_executable(${EXECUTABLE_TARGET} ${SOURCES})

# Link all dependent libraries
target_link_libraries(${EXECUTABLE_TARGET}
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    spdlog::spdlog
    glm::glm
    nlohmann_json::nlohmann_json
    # luacpp::luacpp
)

target_include_directories(${EXECUTABLE_TARGET}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

# ==============================================
# Application configuration
# ==============================================

# Compile shaders from GLSL -> SPV
# compile_shaders(${EXECUTABLE_TARGET})
compile_shaders_v2(${EXECUTABLE_TARGET})

# Set compilation options (defined in CompilerSettings.cmake)
setup_compiler_options(${EXECUTABLE_TARGET})

# Configuration resource file copy (defined in BuildHelpers.cmake)
setup_asset_copy(${EXECUTABLE_TARGET})

# Configure Windows DLL replication (defined in BuildHelpers.cmake)
setup_windows_dll_copy(${EXECUTABLE_TARGET})

# ==============================================
# Print configuration information
# ==============================================

# Print project configuration completion information (defined in ProjectInfo.cmake)
print_project_info(${EXECUTABLE_TARGET})
