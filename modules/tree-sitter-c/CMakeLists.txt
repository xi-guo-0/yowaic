CPMAddPackage(
  NAME tree-sitter-c
  GITHUB_REPOSITORY tree-sitter/tree-sitter-c
  GIT_TAG v0.23.4
)

set(TARGET_NAME tree-sitter-c-modules)
set(SRCS tree-sitter-c.cppm)
add_library(${TARGET_NAME})
target_include_directories(${TARGET_NAME} PUBLIC "${tree-sitter-c_SOURCE_DIR}/bindings/c")
target_link_libraries(${TARGET_NAME} PUBLIC tree-sitter-c)
target_sources(${TARGET_NAME} PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES
  FILES
  ${SRCS}
)
set_source_files_properties(${SRCS} PROPERTIES LANGUAGE CXX)
