
set(TARGET_NAME CSEditor)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Qml Quick QuickControls2 Gui ShaderTools)
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "QT VERSION: ${Qt6_VERSION}")
message(STATUS "QT PATH: ${Qt6_DIR}")

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(${TARGET_NAME} ${SRC_FILES} ${HEADER_FILES})

file(GLOB_RECURSE QML_PATHS LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/*.qml"
)

foreach(filepath IN LISTS QML_PATHS)
    # 将绝对路径转换为相对于当前源码目录的相对路径
    cmake_path(RELATIVE_PATH filepath
        BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE rel_path)
    list(APPEND QML_FILES "${rel_path}")
endforeach()

# message(STATUS "QML文件列表: ${QML_FILES}")
file(GLOB_RECURSE ICON_PATHS LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/*.png"
)

foreach(filepath IN LISTS ICON_PATHS)
    # 将绝对路径转换为相对于当前源码目录的相对路径
    cmake_path(RELATIVE_PATH filepath
        BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE rel_path)
    list(APPEND ICON_FILES "${rel_path}")
endforeach()

# message(STATUS "QML文件列表: ${ICON_FILES}")
set(QML_MODULE CSQML)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_library(${QML_MODULE} STATIC)
qt_add_qml_module(
    ${QML_MODULE}
    URI ${QML_MODULE}
    VERSION 1.0
    QML_FILES ${QML_FILES}
    RESOURCES
    ${ICON_FILES}
    OUTPUT_DIRECTORY ${QML_MODULE}
    RESOURCE_PREFIX /
)

file(GLOB_RECURSE SHADER_PATHS LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
)

foreach(filepath IN LISTS SHADER_PATHS)
    # 将绝对路径转换为相对于当前源码目录的相对路径
    cmake_path(RELATIVE_PATH filepath
        BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE rel_path)
    list(APPEND SHADER_FILES "${rel_path}")
endforeach()

set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders/qsb")

# message(${SHADER_FILES} ${SHADER_OUTPUT_DIR})
qt_add_shaders(${TARGET_NAME} "CS_GL45Shaders"
    GLSL "450"
    PREFIX ""
    FILES ${SHADER_FILES}
    OUTPUT_TARGETS SHADER_LIB
    OUTPUT_DIRECTORY ${SHADER_OUTPUT_DIR}
    PLATFORMS "OpenGL"
)

target_link_libraries(${TARGET_NAME} PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickPrivate
    Qt6::QuickControls2
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::ShaderTools
    ${QML_MODULE}
    ${SHADER_LIB}
    CSEngine
    CSRender
)

if(MSVC)
    target_link_options(${TARGET_NAME} PRIVATE "/WHOLEARCHIVE:$<TARGET_FILE:CSRender>")
endif()

target_include_directories(${TARGET_NAME} PRIVATE ${Qt6Gui_PRIVATE_INCLUDE_DIRS})
target_include_directories(${TARGET_NAME} PRIVATE ${Qt6Quick_PRIVATE_INCLUDE_DIRS})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/meta)

target_compile_definitions(${TARGET_NAME} PRIVATE QT_QML_DEBUG)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_PATH}" "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets"
    COMMENT "Copying files form ${ASSETS_PATH} to $<TARGET_FILE_DIR:${TARGET_NAME}>"
)

if(MSVC)
    get_target_property(QT_QMAKE_EXE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXE}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    # 构建时部署
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --qmldir ${CMAKE_CURRENT_SOURCE_DIR}/qml
            "$<TARGET_FILE:${TARGET_NAME}>"
            --no-compiler-runtime
            WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET_NAME}>"
        )
    endif()
endif()

install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION ${PROJECT_BINARY_PATH}
)
