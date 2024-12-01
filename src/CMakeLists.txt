target_link_libraries(${PROJECT_NAME_LIB} PUBLIC llvm-modules)
target_sources(${PROJECT_NAME_LIB} PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES
  FILES
  lexer.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME_LIB})
target_sources(${PROJECT_NAME} PUBLIC
  main.cpp
)
