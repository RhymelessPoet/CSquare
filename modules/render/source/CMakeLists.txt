set(TARGET_NAME CSRender)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_library(${TARGET_NAME} STATIC ${SRC_FILES} ${HEADER_FILES})

# target_compile_definitions(${TARGET_NAME} PRIVATE -DCS_ENABLE_META)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/graphics/opengl/glad/include)
target_include_directories(${TARGET_NAME} PRIVATE ${THIRD_PARTY_PATH})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/meta)

target_link_libraries(${TARGET_NAME} PRIVATE glfw)
target_link_libraries(${TARGET_NAME} PRIVATE assimp)

target_link_libraries(${TARGET_NAME} PRIVATE CSBase)
target_link_libraries(${TARGET_NAME} PRIVATE CSCore)
target_link_libraries(${TARGET_NAME} PRIVATE Ubpa::UDRefl_core)

if(WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE opengl32 gdi32 user32)
endif()

# ---- Meta code generation ----
set(META_GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)
file(MAKE_DIRECTORY ${META_GENERATED_DIR})

set(META_PARSER ${CMAKE_SOURCE_DIR}/meta/parser/parser.py)
set(META_CONFIG ${CMAKE_SOURCE_DIR}/meta/parser/parse_config.json)
set(META_SCENE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scene)

# Auto-discover headers with potential meta annotations
file(GLOB_RECURSE META_HEADER_FILES CONFIGURE_DEPENDS
    "${META_SCENE_DIR}/*.h"
    "${META_SCENE_DIR}/*.hpp"
)

# Run the meta parser at CONFIGURE time so generated/*.cpp files exist
# before file(GLOB) collects them below. Without this, a fresh
# `cmake --preset MSVC` leaves the generated/ folder empty.
message(STATUS "Running UDRefl meta parser (configure-time)...")
execute_process(
    COMMAND ${Python3_EXECUTABLE} ${META_PARSER}
    ${META_SCENE_DIR}
    ${META_CONFIG}
    --output-dir ${META_GENERATED_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE META_GEN_RESULT
    OUTPUT_VARIABLE META_GEN_STDOUT
    ERROR_VARIABLE META_GEN_STDERR
)

if(NOT META_GEN_RESULT EQUAL 0)
    message(WARNING "Meta parser (configure-time) failed (code=${META_GEN_RESULT}):\n"
        "stdout: ${META_GEN_STDOUT}\n"
        "stderr: ${META_GEN_STDERR}")
else()
    message(STATUS "Meta parser finished successfully.")
endif()

# Stamp file to track when generation was last run (build-time regeneration)
set(META_STAMP ${CMAKE_CURRENT_BINARY_DIR}/meta_gen.stamp)

add_custom_command(
    OUTPUT ${META_STAMP}
    COMMAND ${Python3_EXECUTABLE} ${META_PARSER}
    ${META_SCENE_DIR}
    ${META_CONFIG}
    --output-dir ${META_GENERATED_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${META_STAMP}
    DEPENDS ${META_PARSER}
    ${META_CONFIG}
    ${CMAKE_SOURCE_DIR}/meta/parser/meta_info.py
    ${CMAKE_SOURCE_DIR}/meta/parser/code_generator.py
    ${META_HEADER_FILES}
    COMMENT "Generating UDRefl meta registration code..."
)

add_custom_target(CSRenderMetaGen DEPENDS ${META_STAMP})
add_dependencies(${TARGET_NAME} CSRenderMetaGen)

# Discover generated sources (auto-refreshed when new files appear)
file(GLOB META_GENERATED_SOURCES CONFIGURE_DEPENDS "${META_GENERATED_DIR}/*.cpp")

if(META_GENERATED_SOURCES)
    set_source_files_properties(${META_GENERATED_SOURCES} PROPERTIES GENERATED TRUE)
    target_sources(${TARGET_NAME} PRIVATE ${META_GENERATED_SOURCES})
    message(STATUS "Meta generated sources: ${META_GENERATED_SOURCES}")
else()
    message(STATUS "No meta-annotated classes produced generated sources.")
endif()
