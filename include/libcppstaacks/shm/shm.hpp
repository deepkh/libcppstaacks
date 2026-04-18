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

#pragma once

#include <errno.h>
#include <fcntl.h> // For O_CREAT, O_EXCL
#include <fcntl.h> /* For O_* constants */
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h> // For mode constants
#include <sys/stat.h> /* For mode constants */
#include <unistd.h>   // For sleep()

#include <memory>
#include <span> // since C++ 20
#include <string.h>
#include <string>
#include <vector>

#include <functional>
#include <libcppstaacks/helper/random_data_generator.hpp>

namespace ipc {

class Sem {
public:
  Sem() {}
  ~Sem() { Close(); }

  void Close() {
    if (sem_) {
      // printf("[Sem] %s close\n", sem_name_.c_str());
      sem_close(sem_);
      if (creator_) {
        // printf("[Sem] %s unlink\n", sem_name_.c_str());
        sem_unlink(sem_name_.c_str());
      }
      sem_ = nullptr;
    }
  }

  int Init(const std::string &sem_name, bool creator, int default_value) {
    sem_name_ = sem_name;
    creator_ = creator;
    if (creator_) {
      printf("     [Sem] %s create\n", sem_name_.c_str());
      sem_unlink(sem_name_.c_str());
      sem_ = sem_open(sem_name_.c_str(), O_CREAT | O_EXCL, 0644, default_value);
    } else {
      printf("     [Sem] %s open\n", sem_name_.c_str());
      sem_ = sem_open(sem_name_.c_str(), 0);
    }
    if (sem_ == SEM_FAILED) {
      printf("[Sem] sem error 1\n");
      return -1;
    }
    return 0;
  }

  int Post() { return sem_post(sem_); }
  int Wait() { return sem_wait(sem_); }
  int TryWait() { return sem_trywait(sem_); }
  int GetValue() {
    int value;
    if (sem_getvalue(sem_, &value) == -1) {
      return -1;
    } else {
      printf("[Sem] Semaphore value: %d\n", value); // Should print 3 initially
    }
    return 0;
  }

private:
  bool creator_ = false;
  std::string sem_name_;
  sem_t *sem_ = nullptr;
};

class SemLock {
public:
  SemLock() {}
  ~SemLock() { Close(); }
  void Close() { sem_.Close(); }
  int Init(const std::string &sem_name, bool creator) { return sem_.Init(sem_name, creator, 1); }
  void Lock() { sem_.Wait(); }
  void UnLock() { sem_.Post(); }

private:
  Sem sem_;
};

class ShmAlloc {
public:
  ShmAlloc() {}
  ~ShmAlloc() { Close(); }

  void Close() {
    if (fd_ >= 0) {
      // printf("[ShmAlloc] %s close\n", shm_name_.c_str());
      close(fd_);
      fd_ = -1;

      if (creator_) {
        // printf("[ShmAlloc] %s unlink\n", shm_name_.c_str());
        sem_unlink(shm_name_.c_str());
      }
    }
  }

  int Init(const std::string &shm_name, bool creator, bool reset, int size) {
    void *mapped_addr = nullptr;
    shm_name_ = shm_name;
    creator_ = creator;
    size_ = size;

    if (creator_) {
      // printf("[ShmAlloc] %s create %d\n", shm_name_.c_str(), size_);
      fd_ = shm_open(shm_name_.c_str(), O_CREAT | O_RDWR, 0666);
    } else {
      // printf("[ShmAlloc] %s open %d\n", shm_name_.c_str(), size_);
      fd_ = shm_open(shm_name_.c_str(), O_RDWR, 0666);
    }

    if (fd_ < 0) {
      printf("[ShmAlloc] error 1\n");
      return -1;
    }

    if (creator_ && ftruncate(fd_, size_) == -1) {
      printf("[ShmAlloc] error 2\n");
      return -2;
    }

    if ((mapped_addr = mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0)) ==
        MAP_FAILED) {
      printf("[ShmAlloc] error 3\n");
      return -3;
    }

    mapped_addr_ = static_cast<uint8_t *>(mapped_addr);

    if (creator_ && reset) {
      printf("[ShmAlloc] reset %d to zero\n", size_);
      memset(mapped_addr_, 0, size_);
    }

    printf("[ShmAlloc] fd:%d mapped:%p size:%d\n", fd_, mapped_addr_, size_);
    return 0;
  }

  uint8_t *MappedAddr() { return mapped_addr_; }
  int Size() { return size_; }

private:
  bool creator_ = false;
  std::string shm_name_;
  int fd_ = -1;
  uint8_t *mapped_addr_ = nullptr;
  int size_ = 0;
};

struct ShmDataHead {
  uint64_t seq;
  int size;
};

class ShmData {
public:
  ShmData() {}
  ~ShmData() {}

  void Init(void *addr, int capacity) {
    head_ = static_cast<ShmDataHead *>(addr);
    addr_ = static_cast<uint8_t *>(addr) + sizeof(ShmDataHead);
    capacity_ = capacity;
  }

  ShmDataHead *Head() { return head_; }
  uint8_t *Addr() { return addr_; }
  uint64_t &Seq() { return (head_->seq); }
  int &Size() { return (head_->size); }
  int Capacity() { return capacity_; }

private:
  ShmDataHead *head_ = nullptr;
  uint8_t *addr_ = nullptr;
  int capacity_ = 0;
};

class ShmChunk {
public:
  ShmChunk() {}
  ~ShmChunk() {}

  int Init(const std::string &name, bool creator, void *chunk_addr, int chunk_capacity) {
    name_ = name;
    const std::string sem_lock_name = name_ + "-semlock";
    if (sem_lock_.Init(sem_lock_name, creator)) {
      printf("[ShmChunk] failed to sem_lock_.Init %s %d\n", sem_lock_name.c_str(), creator);
      return -1;
    }

    shm_data_.Init(chunk_addr, chunk_capacity);
    printf("[ShmChunk] %s %p capacity:%d size:%d seq:%lu\n", Name().c_str(), shm_data_.Addr(),
           shm_data_.Capacity(), shm_data_.Size(), shm_data_.Seq());

    return 0;
  }

  void Lock() {
    if (is_lock_) {
      return;
    }
    is_lock_ = true;
    return sem_lock_.Lock();
  }

  void UnLock() {
    if (!is_lock_) {
      return;
    }
    is_lock_ = false;
    sem_lock_.UnLock();
  }

  const std::string &Name() { return name_; }
  ShmData &Data() { return shm_data_; }

private:
  bool is_lock_ = false;
  std::string name_;
  SemLock sem_lock_;
  ShmData shm_data_;
};

struct ShmQueueHead {
  uint64_t write_seq;
  uint64_t read_seq;
  void Reset() {
    write_seq = 0;
    read_seq = 0;
  }
};

class ShmQueue {

public:
  class ShmChunkGuard {
  public:
    ShmChunkGuard(ShmQueue *queue, int i) {
      chunk_ = queue->At(i);
      chunk_->Lock();
    }
    ~ShmChunkGuard() { Dequeue(); }

    ShmData &Data() { return chunk_->Data(); }
    void Dequeue() { chunk_->UnLock(); }

  private:
    ShmQueue *queue = nullptr;
    std::shared_ptr<ShmChunk> chunk_;
  };

  ShmQueue() {}
  ~ShmQueue() {}

  void Close() {
    shm_alloc_.Close();
    shm_chunk_list_.clear();
  }

  int Init(const std::string &shm_name, bool creator, bool reset, int chunk_capacity,
           int chunk_num) {
    const int shm_size = sizeof(ShmQueueHead) + (chunk_capacity + sizeof(ShmDataHead)) * chunk_num;
    printf("%ld %d %ld %d\n", sizeof(ShmQueueHead), chunk_capacity, sizeof(ShmDataHead), chunk_num);
    if (shm_alloc_.Init(shm_name, creator, reset, shm_size)) {
      printf("[ShmQueue] failed to alloc %d\n", shm_size);
      return -1;
    }

    uint8_t *mapped_ptr = shm_alloc_.MappedAddr();
    head_ = static_cast<ShmQueueHead *>((void *)mapped_ptr);
    mapped_ptr += sizeof(ShmQueueHead);

    if (creator && reset) {
      head_->Reset();
    }

    for (int i = 0; i < chunk_num; i++) {
      const std::string chunk_name = shm_name + "-chunk-" + std::to_string(i);
      std::shared_ptr<ShmChunk> shm_chunk = std::make_shared<ShmChunk>();
      if (shm_chunk->Init(chunk_name, creator, mapped_ptr, chunk_capacity)) {
        printf("[ShmQueue] failed to shm_chunk->Init %s\n", chunk_name.c_str());
        return -1;
      }

      shm_chunk_list_.emplace_back(shm_chunk);
      mapped_ptr += chunk_capacity + sizeof(ShmDataHead);
    }

    return 0;
  }

  ShmQueueHead *Head() { return head_; }
  const std::shared_ptr<ShmChunk> &At(int i) { return shm_chunk_list_.at(i); }
  ShmChunkGuard Enqueue(int i) { return ShmChunkGuard(this, i); }
  int Size() { return shm_chunk_list_.size(); }

private:
  ShmQueueHead *head_;
  ShmAlloc shm_alloc_;
  std::vector<std::shared_ptr<ShmChunk>> shm_chunk_list_;
};

} // namespace ipc
