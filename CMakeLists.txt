cmake_minimum_required(VERSION 3.16)
project(shm_test LANGUAGES CXX)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executable target
add_executable(shm_test shm_test.cpp)

# Link pthread, OpenSSL (ssl + crypto)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(shm_test
    PRIVATE
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
)


enable_testing()
find_package(GTest REQUIRED)

add_executable(shm_gtest shm_gtest.cpp)
target_link_libraries(shm_gtest
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
)


enable_testing()
find_package(GTest REQUIRED)

add_executable(sem_gtest sem_gtest.cpp)
target_link_libraries(sem_gtest
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Enable compiler sanitizers (MemorySanitizer)
add_compile_options(-fsanitize=memory -fPIE -pie -fno-omit-frame-pointer -g -O1)
add_link_options(-fsanitize=memory)

add_executable(sanitize_test sanitize_test.cpp)
