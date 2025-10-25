set(PLAYGROUND_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Debug/playground)
file(MAKE_DIRECTORY ${PLAYGROUND_OUTPUT_DIR})

find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)

# Enable compiler sanitizers (MemorySanitizer)
# Shell use clang as compiler
# add_executable(sanitize_test sanitize_test.cpp)
# target_compile_options(sanitize_test PRIVATE -fsanitize=memory -fPIE -pie -fno-omit-frame-pointer -g -O1)
# target_link_options(sanitize_test PRIVATE -fsanitize=memory)
# set_target_properties(sanitize_test PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY ${PLAYGROUND_OUTPUT_DIR}
# )

# sem_gtest
#add_executable(sem_gtest sem_gtest.cpp)
#target_link_libraries(sem_gtest PRIVATE 
#    GTest::GTest GTest::Main 
#    pthread
#    OpenSSL::SSL
#    OpenSSL::Crypto
#)
#set_target_properties(sem_gtest PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY ${CPPSHM_OUTPUT_DIR}
#)
