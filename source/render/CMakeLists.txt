set(TARGET_NAME CSEngine)

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

target_link_libraries(${TARGET_NAME} PRIVATE glfw)