function(setup_asset_copy TARGET_NAME)
    set(COPY_SCRIPT ${CMAKE_SOURCE_DIR}/cmake/scripts/CopyAssets.cmake)

    add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}/assets
            -DTARGET_DIR=$<TARGET_FILE_DIR:${TARGET_NAME}>/assets
            -P ${COPY_SCRIPT}
        COMMENT "Check and copy asset files"
        VERBATIM
    )
endfunction()

function(setup_windows_dll_copy TARGET_NAME)
    if(NOT WIN32)
        return()
    endif()

    if(EXISTS ${CMAKE_SOURCE_DIR}/prebuilt/bin)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/prebuild/bin $<TARGET_FILE_DIR:${TARGET_NAME}>
            COMMENT "Copy prebuilt DLLs to executable directory"
            VERBATIM
        )
    endif()

    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        set(COPY_SCRIPT ${CMAKE_SOURCE_DIR}/cmake/scripts/CopyDLLs.cmake)
        file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/copy_dlls_wrapper_$<CONFIG>.cmake CONTENT "
set(DLL_LIST \"$<TARGET_RUNTIME_DLLS:${TARGET_NAME}>\")
set(TARGET_DIR \"$<TARGET_FILE_DIR:${TARGET_NAME}>\")

include(\"${COPY_SCRIPT}\")
")

        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/copy_dlls_wrapper_$<CONFIG>.cmake
            COMMENT "Auto-detect and copy runtime DLLs"
            VERBATIM
        )

    else()

        message(WARNING "CMake version < 3.21, unable to automatically copy runtime DLLs.")
        message(WARNING "If a dynamic library is used, please manually copy the DLL to the .exe directory.")

    endif()
endfunction()
