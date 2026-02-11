
if(NOT DEFINED TARGET_DIR)
    message(FATAL_ERROR "Required paramter: TARGET_DIR (should be set by caller)")
endif()

if(DEFINED DLL_LIST AND DLL_LIST)
    set(COPIED_COUNT 0)
    set(COPIED_FILES "")

    foreach(DLL IN LISTS DLL_LIST)
        if(NOT EXISTS "${DLL}")
            continue()
        endif()

        get_filename_component(DLL_NAME "${DLL}" NAME)
        set(TARGET_DLL "${TARGET_DIR}/${DLL_NAME}")

        set(NEEDS_COPY FALSE)
        if(EXISTS "${TARGET_DLL}")
            file(MD5 "${DLL}" SRC_MD5)
            file(MD5 "${TARGET_DLL}" DST_MD5)
            if(NOT "${SRC_MD5}" STREQUAL "${DST_MD5}")
                set(NEEDS_COPY TRUE)
            endif()
        else()
            set(NEEDS_COPY TRUE)
        endif()

        if(NEEDS_COPY)
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL}" "${TARGET_DIR}"
                RESULT_VARIABLE COPY_RESULT
                OUTPUT_QUIET
                ERROR_QUIET
            )

            if(COPY_RESULT EQUAL 0)
                math(EXPR COPIED_COUNT "${COPIED_COUNT} + 1")
                list(APPEND COPIED_FILES "  - ${DLL_NAME}")
            else()
                message(WARNING "Failed to copy DLL: ${DLL}")
            endif()

            string(REGEX REPLACE "\\.dll$" ".pbd" PBD_FILE "${DLL}")
            if(EXISTS "${PBD_FILE}")
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PBD_FILE}" "${TARGET_DIR}"
                    OUTPUT_QUIET
                    ERROR_QUIET
                )
            endif()
        endif()
    endforeach()

    if(COPIED_COUNT GREATER 0)
        message(STATUS "Updated ${COPIED_COUNT} runtime DLLs:")
        foreach(FILE IN LISTS COPIED_FILES)
            message(STATUS "${FILE}")
        endforeach()
    endif()

else()

    if(NOT EXISTS "${TARGET_DIR}/.dll_check_done")
        message(STATUS "No runtime DLL dependencies detected (static linking)")
        file(WRITE "${TARGET_DIR}/.dll_check_done" "checked")
    endif()

endif()
