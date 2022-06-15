cmake_minimum_required(VERSION 3.1)

project(carl)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(source)

###

add_executable(carl-test
  test/main.cpp
)

target_link_libraries(carl-test
    CARL
)
