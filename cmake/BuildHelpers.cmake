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
