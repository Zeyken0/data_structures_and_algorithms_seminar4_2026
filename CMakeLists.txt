cmake_minimum_required(VERSION 3.16)

project(ExternalSortingProject
    VERSION 1.0.0
    DESCRIPTION "External sorting algorithms with Catch2 v2 tests"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_subdirectory(external_sorting)
add_subdirectory(tests)
