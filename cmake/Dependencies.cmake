include(FetchContent)

set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# ==============================================
# Auxiliary macros: Find or retrieve dependencies (Smart shallow cloning + link type control)
# Usage:
#
# Parameters:
#   DEP_NAME      - Internal name of the dependency (used for FetchContent and add_subdirectory, etc.)
#   PACKAGE_NAME  - Package name of the dependency (used for find_package to find locally installed packages)
#   GIT_REPO      - Git repository address of the dependency (used by FetchContent to retrieve source code online)
#   GIT_TAG       - Git branch, tag, or commit hash of the dependency (used to specify the version of the source code to retrieve)
#   LOCAL_PATH    - Local source code path (e.g. external/SDL), used for local source code mode
#   LINK_TYPE     - Link type: STATIC / SHARED / AUTO (using global BUILD_SHARED_LIBS)
# ==============================================
macro(find_or_fetch_dependency DEP_NAME PACKAGE_NAME GIT_REPO GIT_TAG LOCAL_PATH LINK_TYPE)
    message(STATUS "Processing dependencies: ${DEP_NAME}")
endmacro()
