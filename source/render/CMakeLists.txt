set(TARGET_NAME CSRender)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_library(${TARGET_NAME} STATIC ${SRC_FILES} ${HEADER_FILES})

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/graphics/opengl/glad/include)
target_include_directories(${TARGET_NAME} PRIVATE ${THIRD_PARTY_PATH})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/meta)

target_link_libraries(${TARGET_NAME} PRIVATE glfw)
target_link_libraries(${TARGET_NAME} PRIVATE assimp)
target_link_libraries(${TARGET_NAME} PRIVATE CSEngine)
target_link_libraries(${TARGET_NAME} PRIVATE Ubpa::UDRefl_core)

# ---- Meta code generation ----
set(META_GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)
file(MAKE_DIRECTORY ${META_GENERATED_DIR})

set(META_PARSER ${CMAKE_SOURCE_DIR}/meta/parser/parser.py)
set(META_CONFIG ${CMAKE_SOURCE_DIR}/meta/parser/parse_config.json)

# Auto-discover headers with potential meta annotations
file(GLOB_RECURSE CONFIGURE_DEPENDS META_HEADER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/scene/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/scene/*.hpp"
)

# Stamp file to track when generation was last run
set(META_STAMP ${CMAKE_CURRENT_BINARY_DIR}/meta_gen.stamp)

add_custom_command(
    OUTPUT ${META_STAMP}
    COMMAND ${Python3_EXECUTABLE} ${META_PARSER}
    ${CMAKE_CURRENT_SOURCE_DIR}/scene
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
file(GLOB CONFIGURE_DEPENDS META_GENERATED_SOURCES "${META_GENERATED_DIR}/*.cpp")

# Fallback: ensure expected files are listed on first configure
if(NOT META_GENERATED_SOURCES)
    set(META_GENERATED_SOURCES
        ${META_GENERATED_DIR}/CSMetaRegister_Auto.cpp
        ${META_GENERATED_DIR}/CSMetaRegister_SceneObject.cpp
        ${META_GENERATED_DIR}/CSMetaRegister_Transform.cpp
    )
endif()

set_source_files_properties(${META_GENERATED_SOURCES} PROPERTIES GENERATED TRUE)
target_sources(${TARGET_NAME} PRIVATE ${META_GENERATED_SOURCES})
