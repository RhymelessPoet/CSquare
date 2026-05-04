set(THIRD_PARTY_FOLDER "ThirdParty")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # 以静态方式链接库

if(NOT TARGET glfw)
    option(GLFW_BUILD_EXAMPLES "Do not build the GLFW example programs" OFF)
    option(GLFW_BUILD_TESTS "Do not build the GLFW test programs" OFF)
    option(GLFW_BUILD_DOCS "Do not build the GLFW documentation" OFF)
    option(GLFW_INSTALL "Do not generate installation target" OFF)
    add_subdirectory(glfw)
    set_target_properties(glfw PROPERTIES FOLDER ${THIRD_PARTY_FOLDER}/glfw)
    set_target_properties(update_mappings PROPERTIES FOLDER ${THIRD_PARTY_FOLDER}/glfw)
endif()

add_subdirectory(assimp)

if(NOT TARGET Ubpa::UDRefl_core)
    add_subdirectory(UDRefl)
    set_target_properties(UDRefl_core PROPERTIES FOLDER ${THIRD_PARTY_FOLDER}/UDRefl)
endif()
