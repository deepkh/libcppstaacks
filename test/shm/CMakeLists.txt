set(CPPSHM_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Debug/)
file(MAKE_DIRECTORY ${CPPSHM_OUTPUT_DIR})

find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)

#add_library(module_a STATIC module_a_lib.cpp)
#target_include_directories(module_a PUBLIC ${CMAKE_SOURCE_DIR}/include)
#set_target_properties(module_a PROPERTIES
#    ARCHIVE_OUTPUT_DIRECTORY ${CPPSHM_OUTPUT_DIR}
#)

# sem_gtest
add_executable(sem_gtest sem_gtest.cpp)
target_link_libraries(sem_gtest PRIVATE 
    GTest::GTest GTest::Main 
    pthread
    OpenSSL::SSL
    OpenSSL::Crypto
)
set_target_properties(sem_gtest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CPPSHM_OUTPUT_DIR}
)

# shm_gtest
add_executable(shm_gtest shm_gtest.cpp)
target_link_libraries(shm_gtest PRIVATE 
    GTest::GTest GTest::Main 
    pthread
    OpenSSL::SSL
    OpenSSL::Crypto
)
set_target_properties(shm_gtest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CPPSHM_OUTPUT_DIR}
)

# shm_test
add_executable(shm_test shm_test.cpp)
target_link_libraries(shm_test PRIVATE 
    GTest::GTest GTest::Main 
    pthread
    OpenSSL::SSL
    OpenSSL::Crypto
)
set_target_properties(shm_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CPPSHM_OUTPUT_DIR}
)