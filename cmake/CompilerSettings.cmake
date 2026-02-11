set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

function(setup_compiler_options TARGET_NAME)
    if(MSVC)
        # Visual Studio: Enable all warnings + UTF-8 encoding support
        target_compile_options(${TARGET_NAME} PRIVATE /W4 /utf-8)
        # Support for parallel compilation
        add_compile_options(/MP)
        # Suppress console window
        # target_link_options(${TARGET_NAME} PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
    elseif(WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
        # MinGW/Clang on Windows: Set UTF-8 encoding
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -finput-charset=utf-8 -fexec-charset=utf-8)
    else()
        # Linux/macOS: Standard warning options
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()
