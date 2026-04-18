#include <cppstuff/helper/ipc_helper.hpp>
#include <cppstuff/shm/shm.hpp>
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
        printf("HI FUCK IM CHILD\n");
        run_reader(0);
        exit(0);
    }

    // I am the parent
    printf("HI FUCK IM PARENT\n");
    run_writer(0);
    ASSERT_EQ(0, ipc::helper::wait_for_child_fork(pid)); // wait for invalid pid
}

// main() entry for GoogleTest
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
