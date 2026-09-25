cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME CircleSquare)
project(${PROJECT_NAME} LANGUAGES CXX VERSION 0.0.0)

option(CS_BUILD_TESTS "Build interactive CSquare test applications" ON)

set(THIRD_PARTY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
set(PROJECT_BINARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 定义资源路径
set(ASSETS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/assets)

find_package(Python3 3.12 COMPONENTS Interpreter REQUIRED)

add_subdirectory(thirdparty)
add_subdirectory(modules)
