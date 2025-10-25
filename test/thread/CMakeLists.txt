set(CPPSHM_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Debug/)
file(MAKE_DIRECTORY ${CPPSHM_OUTPUT_DIR})

find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)

#add_library(module_a STATIC module_a_lib.cpp)
#target_include_directories(module_a PUBLIC ${CMAKE_SOURCE_DIR}/include)
#set_target_properties(module_a PROPERTIES
#    ARCHIVE_OUTPUT_DIRECTORY ${CPPSHM_OUTPUT_DIR}
#)

# msg_queue_gtest
add_executable(msg_queue_gtest msg_queue_gtest.cpp)
target_link_libraries(msg_queue_gtest PRIVATE 
    GTest::GTest GTest::Main 
    pthread
    OpenSSL::SSL
    OpenSSL::Crypto
)
set_target_properties(msg_queue_gtest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CPPSHM_OUTPUT_DIR}
)
