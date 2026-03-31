cmake_minimum_required(VERSION 3.24)
project(blocks)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BINARY_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
make_directory(${BINARY_DIR})

find_program(GLSLC_EXECUTABLE NAMES glslc REQUIRED)
if(APPLE)
    find_program(SHADERCROSS_EXECUTABLE NAMES shadercross)
    find_program(SPIRV_CROSS_EXECUTABLE NAMES spirv-cross)
    if(NOT SHADERCROSS_EXECUTABLE AND NOT SPIRV_CROSS_EXECUTABLE)
        message(FATAL_ERROR "macOS builds require shader translation for Metal. Install shadercross or spirv-cross.")
    endif()
endif()

add_subdirectory(lib/SDL)
add_executable(blocks WIN32
    lib/sqlite3/sqlite3.c
    lib/stb/stb.c
    src/block.c
    src/camera.c
    src/chunk.c
    src/database.c
    src/helpers.c
    src/main.c
    src/noise.c
    src/pipeline.c
    src/raycast.c
    src/voxel.c
    src/world.c
)
target_link_libraries(blocks PUBLIC SDL3::SDL3)
if(UNIX)
    target_link_libraries(blocks PUBLIC m)
endif()
target_include_directories(blocks PUBLIC lib/sqlite3)
target_include_directories(blocks PUBLIC lib/stb)
set_target_properties(blocks PROPERTIES C_STANDARD 11)

function(shader FILE)
    set(SOURCE shaders/${FILE})
    set(SPIRV_OUTPUT ${BINARY_DIR}/${FILE}.spv)
    set(OUTPUTS ${SPIRV_OUTPUT})
    add_custom_command(
        OUTPUT ${SPIRV_OUTPUT}
        COMMAND ${GLSLC_EXECUTABLE} ${SOURCE} -o ${SPIRV_OUTPUT} -I src
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS ${SOURCE} shaders/helpers.glsl src/config.h
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
    string(REPLACE . _ NAME ${FILE})
    add_custom_target(${NAME} DEPENDS ${OUTPUTS})
    add_dependencies(blocks ${NAME})
endfunction()
shader(composite.frag)
shader(fullscreen.vert)
shader(opaque.frag)
shader(opaque.vert)
shader(random.frag)
shader(raycast.frag)
shader(raycast.vert)
shader(shadow.frag)
shader(shadow.vert)
shader(sky.frag)
shader(sky.vert)
shader(ssao.frag)
shader(transparent.frag)
shader(transparent.vert)
shader(ui.frag)

configure_file(LICENSE.txt ${BINARY_DIR} COPYONLY)
configure_file(README.md ${BINARY_DIR} COPYONLY)
configure_file(textures/atlas.png ${BINARY_DIR} COPYONLY)
