target_link_libraries(${PROJECT_NAME_TEST} PUBLIC
  ${PROJECT_NAME_LIB}
  gtest
)
target_sources(${PROJECT_NAME_TEST} PUBLIC
  test_main.cpp
)
target_compile_options(${PROJECT_NAME_TEST} PUBLIC -fno-access-control)
