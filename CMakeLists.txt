# CMakeLists.txt file
cmake_minimum_required(VERSION 3.10)
project(qb-zamai)

# qb minimum requirements
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qb")
option(CMAKE_BUILD_TYPE Release)

# Add qb framework
add_subdirectory(${QB_PATH})
# Add LXUtils
# add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lxUtils")

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lxUtils/src/xstring.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lxUtils/src/xutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
# LX win32 issue
if (WIN32)
add_compile_definitions(WIN32=1)
endif()
target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lxUtils/inc/)
target_link_libraries(${PROJECT_NAME} qb-core)
