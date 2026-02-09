# Runtime Dynamic Library Search Path Configuration Module
# Function: Solves the problem of not being able to find dynamic libraries at runtime.

# Using RPATH during build time eliminates the need to set LD_LIBRARY_PATH for execution
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
