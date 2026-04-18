# libcppstaacks

[![Build and Release](https://github.com/deepkh/libcppstaacks/actions/workflows/release.yml/badge.svg?branch=main)](https://github.com/deepkh/libcppstaacks/actions/workflows/release.yml)

Small C++20 experiments around concurrency and IPC.

The repository currently contains two main building blocks:

- `include/libcppstaacks/thread/msg_queue.hpp`: a thread-safe in-process message queue using `std::queue`, `std::mutex`, `std::condition_variable`, `std::promise`, and `std::future`
- `include/libcppstaacks/shm/shm.hpp`: POSIX shared-memory helpers built from `shm_open`, `mmap`, and named semaphores for simple multi-process data exchange

Most of the project behavior is exercised through GoogleTest-based executables under `test/`.

## Repository layout

```text
include/libcppstaacks/
  helper/
    ipc_helper.hpp
    random_data_generator.hpp
  shm/
    shm.hpp
  thread/
    msg_queue.hpp

test/
  shm/
    sem_gtest.cpp
    shm_gtest.cpp
    shm_test.cpp
  thread/
    msg_queue_gtest.cpp
```

## Requirements

- CMake 3.14+
- C++20 compiler
- GoogleTest
- OpenSSL
- POSIX environment with shared memory and named semaphores support

Tested in this workspace with:

- GCC 14.2.0
- GTest 1.15.0
- OpenSSL 3.5.5

## Build

The checked-in `build/` directory is stale in this repository and may point to a different absolute source path. Use a fresh out-of-tree build instead.

```bash
cmake -S . -B build-local -DCMAKE_BUILD_TYPE=Debug
cmake --build build-local -j"$(nproc)"
```

Artifacts are written to `Debug/` by the top-level CMake configuration.

There is also a helper script:

```bash
./build.sh
```

`build.sh clean` removes `build/` and `Debug/`.

## Run

This project builds test executables, but it does not currently register them with CTest. Run the binaries directly from `Debug/`.

```bash
./Debug/msg_queue_gtest
./Debug/sem_gtest
./Debug/shm_gtest
./Debug/shm_test writer
./Debug/shm_test reader
```

Examples verified in this workspace:

- `./Debug/msg_queue_gtest`
- `./Debug/sem_gtest`
- `./Debug/shm_gtest --gtest_filter=ShmSeqTest.ReaderWriter`

## What Is Implemented

### Thread Message Queue

`cppstuff::thread::MsgQueue<T>` is a blocking producer-consumer queue for shared pointers.

Related type:

- `cppstuff::thread::Msg<R, P>` wraps a request payload plus a promise/future pair for responses

The tests in `test/thread/msg_queue_gtest.cpp` demonstrate:

- async request/response handling between threads
- multiple message types
- high-volume producer/consumer flow

Minimal sketch:

```cpp
using namespace cppstuff::thread;

struct Request {
    int id;
    std::string data;
};

struct Response {
    bool success;
    std::string message;
};

auto message = std::make_shared<Msg<Request, Response>>(1, Request{42, "payload"});
MsgQueue<Msg<Request, Response>> queue;

queue.Push(message);
auto same_message = queue.Pop();
same_message->setResponse(Response{true, "ok"});
auto response = message->getResponse().get();
```

### Shared Memory Queue

`ipc::ShmQueue` manages:

- one shared-memory allocation
- a fixed number of chunks
- one named semaphore lock per chunk
- per-chunk metadata for sequence number and payload size

Related helpers:

- `ipc::Sem`
- `ipc::SemLock`
- `ipc::ShmAlloc`
- `ipc::ShmData`
- `random_data_generator::gen`
- `random_data_generator::md5`

The shared-memory tests cover:

- parent/child synchronization with a named semaphore
- fixed-slot shared-memory writes and reads
- large-buffer integrity checks using MD5

## Current Caveats

- `ctest` reports no tests because `enable_testing()` and `add_test()` are not configured.
- `random_data_generator.hpp` uses OpenSSL `MD5()`, which emits a deprecation warning on OpenSSL 3.x.
- `build.sh` assumes deleting `build/` and `Debug/` is acceptable.
- The codebase is a small IPC/concurrency utility repository rather than a packaged library with install targets or versioned releases.
