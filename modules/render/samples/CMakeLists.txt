# Standalone render sample executables (GLFW, no Qt).
# Each exe links against CSRender + CSEngine and opens its own GLFW window
# driven by SampleModule. Sources in this folder are deliberately excluded
# from the CSRender static library (see ../CMakeLists.txt).

set(SAMPLE_COMMON_SOURCES
    IRenderSample.cpp
    SampleModule.cpp
)

function(cs_add_sample name main_src sample_src)
    add_executable(${name} ${main_src} ${sample_src} ${SAMPLE_COMMON_SOURCES})

    target_include_directories(${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/source/render
        ${CMAKE_SOURCE_DIR}/meta
        ${THIRD_PARTY_PATH}
    )

    target_link_libraries(${name} PRIVATE
        CSBase
        CSCore
        CSRender
        glfw
        Ubpa::UDRefl_core
    )

    if(WIN32)
        target_link_libraries(${name} PRIVATE opengl32 gdi32 user32)
    endif()

    # UDRefl auto-registration translation units in source/render/generated
    # must survive static-library stripping or pre-main registration vanishes.
    if(MSVC)
        target_link_options(${name} PRIVATE "/WHOLEARCHIVE:$<TARGET_FILE:CSRender>")
    endif()

    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSETS_PATH}" "$<TARGET_FILE_DIR:${name}>/assets"
        COMMENT "Copying assets to $<TARGET_FILE_DIR:${name}>/assets"
    )

    # Samples load assets via paths relative to the current working directory
    # (e.g. "assets/hdr/...", "assets/shape/cube.gltf"). Visual Studio's
    # default debugger working directory is $(ProjectDir), which is the
    # samples source dir, not the Debug/Release output dir where assets are
    # copied. Point the debugger at the actual output folder so the sample
    # finds its assets when launched from VS / F5.
    set_target_properties(${name} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${name}>"
    )
endfunction()

cs_add_sample(HelloTriangles main_hello_triangles.cpp SHelloTriangles.cpp)
cs_add_sample(PanoramicHDRSky main_panoramic_hdr_sky.cpp SPanoramicHDRSky.cpp)
cs_add_sample(AssetLoad main_asset_load.cpp SAssetLoad.cpp)
cs_add_sample(InfiniteGrid3D main_infinite_grid_3d.cpp SInfiniteGrid3D.cpp)

# Expose the sample target list to sibling CMake trees (e.g. the editor)
# so they can both depend on these targets and deploy their executables
# alongside the editor binary. CACHE INTERNAL keeps it out of the UI but
# visible to every subdirectory regardless of scope ordering.
set(CS_SAMPLE_TARGETS
    HelloTriangles
    PanoramicHDRSky
    AssetLoad
    InfiniteGrid3D
    CACHE INTERNAL "Render sample executable targets" FORCE)
