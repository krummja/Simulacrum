include(FetchContent)

set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# ==============================================
# Auxiliary macros: Find or retrieve dependencies (Smart shallow cloning + link type
#                   control)
#
# Parameters:
#   DEP_NAME      - Internal name of the dependency (used for FetchContent and
#                   add_subdirectory, etc.)
#   PACKAGE_NAME  - Package name of the dependency (used for find_package to find locally
#                   installed packages)
#   GIT_REPO      - Git repository address of the dependency (used by FetchContent to
#                   retrieve source code online)
#   GIT_TAG       - Git branch, tag, or commit hash of the dependency (used to specify
#                   the version of the source code to retrieve)
#   LOCAL_PATH    - Local source code path (e.g. external/SDL), used for local source
#                   code mode
#   LINK_TYPE     - Link type: STATIC / SHARED / AUTO (using global BUILD_SHARED_LIBS)
# ==============================================
macro(find_or_fetch_dependency
    DEP_NAME
    PACKAGE_NAME
    GIT_REPO
    GIT_TAG
    LOCAL_PATH
    LINK_TYPE
)
    message(STATUS "Processing dependencies: ${DEP_NAME}")

    # Determine the library link type
    if("${LINK_TYPE}" STREQUAL "STATIC")
        set(_LIB_IS_SHARED OFF)
        set(_LINK_TYPE_STR "Static")
    elseif()
        set(_LIB_IS_SHARED ON)
        set(_LINK_TYPE_STR "Dynamic")
    else() # AUTO or other values, using global settings
        set(_LIB_IS_SHARED ${BUILD_SHARED_LIBS})
        if(BUILD_SHARED_LIBS)
            set(_LINK_TYPE_STR "Dynamic (global)")
        else()
            set(_LINK_TYPE_STR "Static (global)")
        endif()
    endif()

    # First try to find locally installed packages
    find_package(${PACKAGE_NAME} QUIET)

    if(${PACKAGE_NAME}_FOUND OR ${DEP_NAME}_FOUND)
        message(STATUS "  ✓ Found local installation")

        # Print package path information (try multiple possible variables)
        if(DEFINED ${PACKAGE_NAME}_DIR)
            message(STATUS "    Path: ${${PACKAGE_NAME}_DIR}")
        elseif(DEFINED ${DEP_NAME}_DIR)
            message(STATUS "    Path: ${${DEP_NAME}_DIR}")
        elseif(DEFINED ${PACKAGE_NAME}_ROOT)
            message(STATUS "    Root Directory: ${${PACKAGE_NAME}_ROOT}")
        elseif(DEFINED ${DEP_NAME}_ROOT)
            message(STATUS "    Root Directory: ${${DEP_NAME}_ROOT}")
        elseif(DEFINED ${PACKAGE_NAME}_INCLUDE_DIRS)
            message(STATUS "    Header Files: ${${PACKAGE_NAME}_INCLUDE_DIRS}")
        elseif(DEFINED ${DEP_NAME}_INCLUDE_DIRS)
            message(STATUS "    Header Files: ${${DEP_NAME}_INCLUDE_DIRS}")
        endif()
    else()
        message(STATUS "  ✗ No local installation found. Ready to build from source [${_LINK_TYPE_STR}]")

        # Configure library-specific compilation options (before add_subdirectory or
        # FetchContent)

        # Options specific to SDL series libraries
        if("${DEP_NAME}" MATCHES "^SDL" OR "${PACKAGE_NAME}" MATCHES "^SDL")
            if(_LIB_IS_SHARED)
                set(SDL_SHARED ON CACHE BOOL "" FORCE)
                set(SDL_STATIC OFF CACHE BOOL "" FORCE)
            else()
                set(SDL_SHARED OFF CACHE BOOL "" FORCE)
                set(SDL_STATIC ON CACHE BOOL "" FORCE)
            endif()

            # Disable SDL testing
            set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
            set(SDL_TESTS OFF CACHE BOOL "" FORCE)
            set(SDL_INSTALL_TESTS OFF CACHE BOOL "" FORCE)
        endif()

        # Common options for other libraries
        set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_DOCS OFF CACHE BOOL "" FORCE)

        # SDL_image specific option: Disable format support that may cause build issues
        if("${DEP_NAME}" STREQUAL "SDL3_image")
            # Disable AVIF format (requires complex toolchains such as NASM and Meson)
            set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AVIF_SHARED OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AVIF_SAVE OFF CACHE BOOL "" FORCE)
            # Disable AVIF's dependency library
            set(SDLIMAGE_AVIF_VENDORED OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_DAV1D OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AOM OFF CACHE BOOL "" FORCE)
            # Optional: Disable other formats that may have build issues
            # set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)  # JPEG XL
        endif()

        # Smart Selection: Prioritize local source code, otherwise get it online
        set(LOCAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${LOCAL_PATH})
        if(EXISTS ${LOCAL_SOURCE_DIR})
            # Local source code detected, use local compilation
            message(STATUS "  → Use local source code: ${LOCAL_SOURCE_DIR}")

            # Temporarily setting BUILD_SHARED_LIBS affects this library
            set(_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
            set(BUILD_SHARED_LIBS ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)

            # add_subdirectory parameter description:
            # 1. Source code directory (${LOCAL_SOURCE_DIR}), which is the local source
            #    code path of the dependent library
            # 2. The binary output directory (${CMAKE_BINARY_DIR}/_deps/${DEP_NAME}-build);
            #    that is, the build output of the dependent library will be placed here to
            #    avoid polluting the build directory of the main project
            add_subdirectory(${LOCAL_SOURCE_DIR} ${CMAKE_BINARY_DIR}/_deps/${DEP_NAME}-build)

            # Restore global settings
            set(BUILD_SHARED_LIBS ${_SAVED_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)

            # Ensure namespace aliases exist (compatibility handling)
            # Some libraries may not automatically create ALIAS targets with namespaces
            # when compiled from source code
            # Here we create aliases in the PACKAGE_NAME::PACKAGE_NAME format uniformly
            if(TARGET ${DEP_NAME} AND NOT TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
                message(STATUS "     Creating alias: ${PACKAGE_NAME}::${PACKAGE_NAME} -> ${DEP_NAME}")
                add_library(${PACKAGE_NAME}::${PACKAGE_NAME} ALIAS ${DEP_NAME})
            elseif(TARGET ${PACKAGE_NAME} AND NOT TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
                message(STATUS "     Creating alias: ${PACKAGE_NAME}::${PACKAGE_NAME} -> ${PACKAGE_NAME}")
                add_library(${PACKAGE_NAME}::${PACKAGE_NAME} ALIAS ${PACKAGE_NAME})
            endif()

        else()
            # The local source code does not exist; use FetchContent to get it online
            message(STATUS "  → Local source code does not exist; get it online: ${GIT_REPO}")

            # Intellgently detect GIT_TAG type and decide whether to use shallow cloning
            string(LENGTH "${GIT_TAG}" TAG_LENGTH)
            string(REGEX MATCH "^[0-9a-f]+$" IS_HEX "${GIT_TAG}")

            # Determine whether it is a commit hash (40-bit hexadecimal string)
            if(TAG_LENGTH EQUAL 40 AND IS_HEX)
                message(STATUS "     Version: commit hash [${GIT_TAG}], complete clone")
                set(USE_SHALLOW FALSE)
            else()
                message(STATUS "     Version: ${GIT_TAG}, shallow clone")
                set(USE_SHALLOW TRUE)
            endif()

            # FetchContent_Declare is used to declare a FetchContent object for obtaining
            # dependency library source code online
            # Parameter description:
            #   DEP_NAME       - Dependent internal names (for FetchContent and
            #                    add_subcirectory, etc.)
            #   GIT_REPOSITORY - Dependent Git repository address
            #   GIT_TAG        - Dependent Git branch, tag, or commit hash
            #   GIT_SHALLOW    - Whether ot use shallow cloning (TRUE/FALSE),
            #                    automatically determined based on GIT_TAG type
            #   GIT_PROGRESS   - Whether to display Git progress (TRUE/FALSE)

            # Temporarily setting BUILD_SHARED_LIBS affects this library
            set(_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
            set(BUILD_SHARED_LIBS ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)

            FetchContent_Declare(
                ${DEP_NAME}
                GIT_REPOSITORY ${GIT_REPO}
                GIT_TAG ${GIT_TAG}
                GIT_SHALLOW ${GIT_SHALLOW}
                GIT_PROGRESS TRUE
            )

            FetchContent_MakeAvailable(${DEP_NAME})

            # Restore global settings
            set(BUILD_SHARED_LIBS ${_SAVED_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
        endif()
    endif()
endmacro()


# ==============================================
# Setup all project dependencies
# ==============================================
function(setup_project_dependencies)
    # SDL3
    find_or_fetch_dependency(
        SDL3
        SDL3
        "https://github.com/libsdl-org/SDL.git"
        "release-3.2.24"
        "external/SDL-release-3.2.24"
        AUTO  # Use global BUILD_SHARED_LIBS settings
    )

    # SDL3_image
    find_or_fetch_dependency(
        SDL3_image
        SDL3_image
        "https://github.com/libsdl-org/SDL_image.git"
        "release-3.2.4"
        "external/SDL_image-release-3.2.4"
        AUTO
    )

    # SDL3_mixer
    find_or_fetch_dependency(
        SDL3_mixer
        SDL3_mixer
        "https://github.com/libsdl-org/SDL_mixer.git"
        "30c1301055a35ee87b8679279b6fc88e10d28fa3"
        "external/SDL_mixer-30c1301"
        AUTO
    )

    find_or_fetch_dependency(
        glm
        glm
        "https://github.com/g-truc/glm.git"
        "1.0.1"
        "external/glm-1.0.1"
        STATIC
    )

    # nlohmann-json (header-only library)
    # Note: FetchContent internal name uses json, but find_package uses nlohmann_json
    find_or_fetch_dependency(
        json
        nlohmann_json
        "https://github.com/nlohmann/json.git"
        "v3.12.0"
        "external/json-3.12.0"
        STATIC  # Header-only library, does not actually affect
    )

    # spdlog
    find_or_fetch_dependency(
        spdlog
        spdlog
        "https://github.com/gabime/spdlog.git"
        "v1.15.3"
        "external/spdlog-1.15.3"
        STATIC  # Recommend static linking to avoid runtime dependencies
    )
endfunction()
