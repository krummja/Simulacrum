set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS ON)
set(SUPPRESS_CONSOLE_WINDOW OFF)

function(setup_compiler_options TARGET_NAME)
    if(MSVC)
        # Visual Studio: Enable all warnings + UTF-8 encoding support
        target_compile_options(${TARGET_NAME} PRIVATE
            /W4
            /utf-8
            /std=c++latest
            /Zi  # program database
        )

        # Support for parallel compilation
        add_compile_options(/MP)

        # Suppress console window
        if (SUPPRESS_CONSOLE_WINDOW)
            target_link_options(${TARGET_NAME} PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
        endif()
    elseif(WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
        # MinGW/Clang on Windows: Set UTF-8 encoding
        target_compile_options(
            ${TARGET_NAME}
            PRIVATE
                -Wall                   # all warnings
                -Wextra                 # extra warnings
                -Wpedantic              # strict ISO C++ warnings
                -finput-charset=utf-8   # input character encoding
                -fexec-charset=utf-8    # execuction character encoding
                -std=c++20
        )
    else()
        # Linux/macOS: Standard warning options
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()
