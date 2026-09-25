set(TARGET_NAME CSComputePipelineTest)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
add_executable(${TARGET_NAME}
    main.cpp
    ComputeTestController.cpp
    ComputeTestController.h
    ComputeResultItem.cpp
    ComputeResultItem.h
    EdgePreservingFilterCase.cpp
    EdgePreservingFilterCase.h
    LaplacianSmoothingCase.cpp
    LaplacianSmoothingCase.h
)

set_target_properties(${TARGET_NAME} PROPERTIES AUTOMOC ON)

qt_add_qml_module(${TARGET_NAME}
    URI CSComputePipelineTest
    VERSION 1.0
    QML_FILES Main.qml
    RESOURCE_PREFIX /
    NO_GENERATE_QMLTYPES
)

target_link_libraries(${TARGET_NAME} PRIVATE
    CSBase
    CSRender
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

set_target_properties(${TARGET_NAME} PROPERTIES
    FOLDER "Tests/Render"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_PATH}/tests/render/$<CONFIG>"
)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets/images"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ASSETS_PATH}/images/lighthouse.png"
    "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets/images/lighthouse.png"
    COMMENT "Deploying ComputePipeline test assets"
)

if(WIN32)
    get_target_property(QT_QMAKE_EXE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXE}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --qmldir "${CMAKE_CURRENT_SOURCE_DIR}"
            "$<TARGET_FILE:${TARGET_NAME}>"
            --no-compiler-runtime
            WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET_NAME}>"
            COMMENT "Deploying Qt runtime for ${TARGET_NAME}"
        )
    else()
        message(WARNING "windeployqt was not found; ${TARGET_NAME} will require a configured Qt runtime environment")
    endif()
endif()
