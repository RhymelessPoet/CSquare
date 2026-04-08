cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME CircleSquare)
project(${PROJECT_NAME} LANGUAGES CXX VERSION 0.0.0)

set(THIRD_PARTY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
set(PROJECT_BINARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 定义资源路径
set(ASSETS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/assets)

find_package(Python3 3.12 COMPONENTS Interpreter REQUIRED)

add_subdirectory(thirdparty)
add_subdirectory(source)

# set(TARGET_NAME MetaGen)
# add_executable(${TARGET_NAME} main.cpp meta/CSMetaExample.cpp)

# add_custom_command(
# TARGET ${TARGET_NAME} PRE_BUILD
# COMMAND ${Python3_EXECUTABLE} ../meta/parser/parser.py ../meta/CSMetaExample.cpp ../meta/parser/parse_config.json
# DEPENDS mata/parser.py meta/CSMetaExample.cpp
# COMMENT "Start generating meta... "
# )