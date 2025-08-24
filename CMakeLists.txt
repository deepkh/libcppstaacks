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