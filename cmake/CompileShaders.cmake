function(compile_shaders TARGET_NAME)
    find_program(GLSLANG_VALIDATOR
        NAMES glslangValidator glslang
        HINTS ENV VULKAN_SDK
        PATH_SUFFIXES bin
    )

    if(NOT GLSLANG_VALIDATOR)
        message(FATAL_ERROR "glslangValidator not found. Install glslang package or Vulkan SDK.")
    endif()

    message(STATUS "Found glslangValidator: ${GLSLANG_VALIDATOR}")

    set(SHADER_SOURCES
        ${CMAKE_SOURCE_DIR}/res/shaders/sprite.vert.glsl
        ${CMAKE_SOURCE_DIR}/res/shaders/sprite.frag.glsl
        ${CMAKE_SOURCE_DIR}/res/shaders/color.vert.glsl
        ${CMAKE_SOURCE_DIR}/res/shaders/color.frag.glsl
        ${CMAKE_SOURCE_DIR}/res/shaders/composite.vert.glsl
        ${CMAKE_SOURCE_DIR}/res/shaders/composite.frag.glsl
    )

    # Compile each shader to SPIR-V
    set(SHADER_SPIRV_OUTPUTS "")

    foreach(SHADER_SOURCE ${SHADER_SOURCES})
        get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME)
        string(REPLACE ".glsl" ".spv" SHADER_SPV_NAME ${SHADER_NAME})
        set(SHADER_SPV "${CMAKE_SOURCE_DIR}/res/shaders/${SHADER_SPV_NAME}")

        # GLSL -> SPV
        execute_process(
            COMMAND ${GLSLANG_VALIDATOR} -V ${SHADER_SOURCE} -o ${SHADER_SPV}
        )

        list(APPEND SHADER_SPIRV_OUTPUTS ${SHADER_SPIRV})
    endforeach()

    message(STATUS ${SHADER_SPIRV_OUTPUTS})

    add_custom_target(compile_shaders ALL
        DEPENDS ${SHADER_SPIRV_OUTPUTS}
    )

    message(STATUS "SDL3_GPU enabled - shaders will be compiled to SPIR-V")

    add_dependencies(${TARGET_NAME} compile_shaders)
endfunction()