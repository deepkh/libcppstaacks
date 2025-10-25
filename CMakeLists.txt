cmake_minimum_required(VERSION 3.14)
project(project_a LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# used for santize_test.cpp
# set(CMAKE_C_COMPILER clang)
# set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Debug)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(test)

#add_executable(main main.cpp)
#target_link_libraries(main PRIVATE module_a module_b)
