find_package (Threads)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
llvm_map_components_to_libnames(llvm_libs support core irreader)
add_definitions(${LLVM_DEFINITIONS})

set(TARGET_NAME llvm-modules)
set(SRCS llvm.cppm)
add_library(${TARGET_NAME})
target_include_directories(${TARGET_NAME} PUBLIC ${LLVM_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE ${llvm_libs})
target_sources(${TARGET_NAME} PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES
  FILES
  ${SRCS}
)
set_source_files_properties(${SRCS} PROPERTIES LANGUAGE CXX)
