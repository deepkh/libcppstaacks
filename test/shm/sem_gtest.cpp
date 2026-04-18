/*
Copyright (c) 2025, 2026 Gary Huang (deepkh@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <libcppstaacks/helper/ipc_helper.hpp>
#include <libcppstaacks/shm/shm.hpp>
#include <gtest/gtest.h>

#include <sys/wait.h> // WIFEXITED, etc.
#include <unistd.h>   // fork()

class SemTest : public ::testing::Test {
  public:
    std::string SEM_NAME = "/my_sem";
    int SEM_RUN = 10000;

    int run_reader(int pid);
    int run_writer(int pid);
};

int SemTest::run_reader(int pid) {
    ipc::SemLock sem_lock;

    usleep(500000);

    if (sem_lock.Init(SEM_NAME, false)) {
        perror("run_writer sem.Init()");
        return -1;
    }

    for (int i = 0; i < SEM_RUN; i++) {
        sem_lock.Lock();
        if (i % 1000 == 0) {
            printf("%s %d entered lock\n", __func__, i);
        }
        sem_lock.UnLock();
    }

    return 0;
}

int SemTest::run_writer(int pid) {
    ipc::SemLock sem_lock;

    if (sem_lock.Init(SEM_NAME, true)) {
        perror("run_writer sem.Init()");
        return -1;
    }

    for (int i = 0; i < SEM_RUN; i++) {
        sem_lock.Lock();
        if (i == 0) {
            usleep(10000000);
        } else if (i % 1000 == 0) {
            printf("%s %d entered lock \n", __func__, i);

            usleep(100000);
        }
        sem_lock.UnLock();
    }

    return 0;
}

TEST_F(SemTest, SemWriterReader) {
    int pid = fork();
    if (0 == pid) { // I am the child
        printf("HI IM CHILD\n");
        run_reader(0);
        exit(0);
    }

    // I am the parent
    printf("HI IM PARENT\n");
    run_writer(0);
    ASSERT_EQ(0, ipc::helper::wait_for_child_fork(pid)); // wait for invalid pid
}

// main() entry for GoogleTest
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
