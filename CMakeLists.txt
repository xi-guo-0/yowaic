cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_MODULE_STD ON)
# https://gitlab.kitware.com/cmake/cmake/-/issues/25965#note_1523575
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  # This specific value changes as experimental support evolves. See
  # `Help/dev/experimental.rst` in the CMake source corresponding to
  # your CMake build for the exact value to use.
  "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(yowaic LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
    "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info
include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)

add_subdirectory(modules/llvm)
add_subdirectory(modules/tree-sitter)
add_subdirectory(modules/tree-sitter-c)

add_executable(${PROJECT_NAME})
set(PROJECT_NAME_LIB "${PROJECT_NAME}lib")
add_library(${PROJECT_NAME_LIB})
add_subdirectory(src)

include(CTest)
if(BUILD_TESTING)
  CPMAddPackage(
    NAME GoogleTest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG release-1.11.0
  )
  enable_testing()
  set(PROJECT_NAME_TEST "${PROJECT_NAME}_test" )
  add_executable(${PROJECT_NAME_TEST})
  add_test(NAME ${PROJECT_NAME_TEST} COMMAND ${PROJECT_NAME_TEST})
  add_subdirectory(test)
endif()
