CPMAddPackage(
  NAME tree-sitter
  GITHUB_REPOSITORY tree-sitter/tree-sitter
  GIT_TAG v0.24.5
  DOWNLOAD_ONLY True
)

if(tree-sitter_ADDED)
  add_subdirectory("${tree-sitter_SOURCE_DIR}/lib" ${tree-sitter_BINARY_DIR} )
endif()

set(TARGET_NAME tree-sitter-modules)
set(SRCS tree-sitter.cppm)
add_library(${TARGET_NAME})
target_include_directories(${TARGET_NAME} PUBLIC "${tree-sitter_SOURCE_DIR}/lib/include")
target_link_libraries(${TARGET_NAME} PUBLIC tree-sitter)
target_sources(${TARGET_NAME} PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES
  FILES
  ${SRCS}
)
set_source_files_properties(${SRCS} PROPERTIES LANGUAGE CXX)
