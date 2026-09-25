add_subdirectory(source)
add_subdirectory(samples)

if(CS_BUILD_TESTS)
    add_subdirectory(test)
endif()