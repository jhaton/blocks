cmake_minimum_required(VERSION 3.24)
project(sdl_gpu_fps_starter C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(BINARY_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
file(MAKE_DIRECTORY ${BINARY_DIR})

find_program(GLSLC_EXECUTABLE NAMES glslc REQUIRED)
if(APPLE)
    find_program(SHADERCROSS_EXECUTABLE NAMES shadercross)
    find_program(SPIRV_CROSS_EXECUTABLE NAMES spirv-cross)
    if(NOT SHADERCROSS_EXECUTABLE AND NOT SPIRV_CROSS_EXECUTABLE)
        message(FATAL_ERROR "macOS builds require shader translation for Metal. Install shadercross or spirv-cross.")
    endif()
endif()

add_subdirectory(lib/SDL)

add_executable(sdl_gpu_fps_starter WIN32
    src/camera.c
    src/helpers.c
    src/main.c
    src/math3d.c
    src/mesh.c
    src/render_pass_overlay.c
    src/render_pass_scene.c
    src/render_pass_shadow.c
    src/renderer.c
    src/save.c
    src/scene.c
    src/shader.c
)

target_link_libraries(sdl_gpu_fps_starter PUBLIC SDL3::SDL3)
if(UNIX)
    target_link_libraries(sdl_gpu_fps_starter PUBLIC m)
endif()

function(shader FILE)
    set(SOURCE shaders/${FILE})
    set(SPIRV_OUTPUT ${BINARY_DIR}/${FILE}.spv)
    set(OUTPUTS ${SPIRV_OUTPUT})
    add_custom_command(
        OUTPUT ${SPIRV_OUTPUT}
        COMMAND ${GLSLC_EXECUTABLE} ${SOURCE} -o ${SPIRV_OUTPUT} -I src
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS ${SOURCE}
        COMMENT ${SOURCE}
    )
    if(APPLE)
        set(MSL_OUTPUT ${BINARY_DIR}/${FILE}.msl)
        if(SHADERCROSS_EXECUTABLE)
            add_custom_command(
                OUTPUT ${MSL_OUTPUT}
                COMMAND ${SHADERCROSS_EXECUTABLE} ${SPIRV_OUTPUT} -o ${MSL_OUTPUT}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                DEPENDS ${SPIRV_OUTPUT}
                COMMENT ${SOURCE}.msl
            )
        else()
            add_custom_command(
                OUTPUT ${MSL_OUTPUT}
                COMMAND ${SPIRV_CROSS_EXECUTABLE} ${SPIRV_OUTPUT} --msl --output ${MSL_OUTPUT}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                DEPENDS ${SPIRV_OUTPUT}
                COMMENT ${SOURCE}.msl
            )
        endif()
        list(APPEND OUTPUTS ${MSL_OUTPUT})
    endif()
    string(REPLACE "." "_" TARGET_NAME ${FILE})
    add_custom_target(${TARGET_NAME} DEPENDS ${OUTPUTS})
    add_dependencies(sdl_gpu_fps_starter ${TARGET_NAME})
endfunction()

shader(fullscreen.vert)
shader(overlay.frag)
shader(scene.frag)
shader(scene.vert)
shader(shadow.frag)
shader(shadow.vert)

configure_file(LICENSE.txt ${BINARY_DIR} COPYONLY)
configure_file(README.md ${BINARY_DIR} COPYONLY)
