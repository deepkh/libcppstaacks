#pragma once

#include <functional>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace ipc {
namespace helper {

//
// https://github.com/google/googletest/issues/1153#issuecomment-428247477
//
static int wait_for_child_fork(int pid) {
    int status;
    if (0 > waitpid(pid, &status, 0)) {
        std::cerr << "[----------]  Waitpid error!" << std::endl;
        return (-1);
    }
    if (WIFEXITED(status)) {
        const int exit_status = WEXITSTATUS(status);
        if (exit_status != 0) {
            std::cerr << "[----------]  Non-zero exit status " << exit_status << " from test!"
                      << std::endl;
        }
        return exit_status;
    } else {
        std::cerr << "[----------]  Non-normal exit from child!" << std::endl;
        return (-2);
    }
    return 0;
}

} // namespace helper
} // namespace ipc