# Copyright (c) 2025, 2026 Gary Huang (deepkh@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

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
