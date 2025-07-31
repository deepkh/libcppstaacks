#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>      // For O_CREAT, O_EXCL
#include <sys/stat.h>   // For mode constants
#include <unistd.h>     // For sleep()
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <string.h>
#include <string>
#include <memory>
#include <vector>

class Sem {
public:
  Sem() {}

  ~Sem() {
    Close();
  }

  void Close() {
    if (sem_) {
      printf("[Sem] %s close\n", sem_name_.c_str());
      sem_close(sem_);
      if (creator_) {
        printf("[Sem] %s unlink\n", sem_name_.c_str());
        sem_unlink(sem_name_.c_str());
      }
      sem_ = nullptr;
    }
  }

  int Init(std::string sem_name, bool creator, int default_value) {
    sem_name_ = sem_name;
    creator_ = creator;
    if (creator_) {
      printf("[Sem] %s create\n", sem_name_.c_str());
      sem_unlink(sem_name_.c_str());
      sem_ = sem_open(sem_name_.c_str(), O_CREAT | O_EXCL, 0644, default_value);
    } else {
      printf("[Sem] %s open\n", sem_name_.c_str());
      sem_ = sem_open(sem_name_.c_str(), 0);
    }
    if (sem_ == SEM_FAILED) {
        printf("[Sem] sem error 1\n");
        return -1;
    }
    return 0;
  }
  
  int Post() {
    return sem_post(sem_);
  }

  int Wait() {
    return sem_wait(sem_);
  }
  
  int TryWait() {
    return sem_trywait(sem_);
  }

  int GetValue() {
    int value;
    if (sem_getvalue(sem_, &value) == -1) {
        return -1;
    } else {
        printf("[Sem] Semaphore value: %d\n", value);  // Should print 3 initially
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

  void Close() {
    sem_.Close();
  }

  int Init(std::string sem_name, bool creator) {
    return sem_.Init(sem_name, creator, 1);
  }

  void Lock() {
    sem_.Wait();
  }

  void UnLock() {
    sem_.Post();
  }

private:
  Sem sem_;
};

class ShmAlloc {
public:
  ShmAlloc() {}

  ~ShmAlloc() {
    Close();
  }

  void Close() {
    if (fd_ >= 0) {
      printf("[ShmAlloc] %s close\n", shm_name_.c_str());
      close(fd_);
      fd_ = -1;

      if (creator_) {
        printf("[ShmAlloc] %s unlink\n", shm_name_.c_str());
        sem_unlink(shm_name_.c_str());
      }
    }
  }

  int Init(std::string shm_name, bool creator, int capacity) {
    shm_name_ = shm_name;
    creator_ = creator;
    capacity_ = capacity;

    if (creator_) {
      printf("[ShmAlloc] %s create %d\n", shm_name_.c_str(), capacity_);
      fd_ = shm_open(shm_name_.c_str(), O_CREAT | O_RDWR, 0666);
    } else {
      printf("[ShmAlloc] %s open %d\n", shm_name_.c_str(), capacity_);
      fd_ = shm_open(shm_name_.c_str(), O_RDWR, 0666);
    }

    if (fd_ < 0) {
      printf("[ShmAlloc] error 1\n");
      return -1;
    }
      
    if (creator_ && ftruncate(fd_, capacity_) == -1) {
      printf("[ShmAlloc] error 2\n");
      return -2;
    }

    if ((mapped_addr_ = mmap(NULL, capacity_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0)) == MAP_FAILED) {
      printf("[ShmAlloc] error 3\n");
      return -3;
    }

    return 0;
  }
  
  void *MappedAddr() {
    return mapped_addr_;
  }

  int Capacity() {
    return capacity_;
  }

private:
  bool creator_ = false;
  std::string shm_name_;
  int fd_ = -1;
  void *mapped_addr_ = nullptr;
  int capacity_ = 0;
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
    data_ = static_cast<uint8_t *>(addr + sizeof(ShmDataHead));
    capacity_ = capacity - sizeof(ShmDataHead);
  }

  uint8_t *Data() {
    return data_;
  }

  uint64_t Seq() {
    return head_->seq;
  }

  int Size() {
    return head_->size;
  }

  int Capacity() {
    return capacity_;
  }

private:
  ShmDataHead *head_ = nullptr;
  uint8_t *data_ = nullptr;
  int capacity_ = 0;
};

class ShmChunk {
public:
  ShmChunk() {}
  ~ShmChunk() {}
  int Init(const std::string &name, bool creator, void *chunk_addr, int chunk_capacity) {
    name_ = name;
    const std::string sem_lock_name = name_ +"-semlock";
    if (sem_lock_.Init(sem_lock_name, creator)) {
      printf("[ShmChunk] failed to sem_lock_.Init %s %d\n", sem_lock_name.c_str(), creator);
      return -1;
    }

    shm_data_.Init(chunk_addr, chunk_capacity);
    printf("[ShmChunk] %s %p %d %d \n"
      , Name().c_str()
      , shm_data_.Data()
      , shm_data_.Capacity()
      , shm_data_.Size());
  }

  const std::string &Name() {
    return name_;
  }

  void Lock() {
    return sem_lock_.Lock();
  }

  void UnLock() {
    sem_lock_.UnLock();
  }

  ShmData &Data() {
    return shm_data_;
  }

private:
  std::string name_;
  SemLock sem_lock_;
  ShmData shm_data_;
};

class ShmQueue {
public:
  ShmQueue() {}
  ~ShmQueue() {}

  void Close() {
    shm_alloc_.Close();
    shm_chunk_list_.clear();
  }

  int Init(const std::string &shm_name, bool creator, int chunk_capacity, int chunk_num) {
    const int shm_capacity = chunk_capacity * chunk_num;
    if (shm_alloc_.Init(shm_name, creator, shm_capacity)) {
      printf("[ShmQueue] failed to alloc %d\n", shm_capacity);
      return -1;
    }

    void *mapped_ptr = shm_alloc_.MappedAddr();

    for (int i=0; i<chunk_num; i++) {
      const std::string chunk_name = shm_name +"-chunk-" + std::to_string(i);
      std::shared_ptr<ShmChunk> shm_chunk = std::make_shared<ShmChunk>();
      if (shm_chunk->Init(chunk_name, creator, mapped_ptr+i*chunk_capacity, chunk_capacity)) {
        printf("[ShmQueue] failed to shm_chunk->Init %s\n", chunk_name.c_str());
        return -1;
      }

      shm_chunk_list_.emplace_back(shm_chunk);
    }
  }

  const std::shared_ptr<ShmChunk> At(int i) {
    return shm_chunk_list_.at(i);
  }

  int Size() {
    return shm_chunk_list_.size();
  }

private:
  ShmAlloc shm_alloc_;
  std::vector<std::shared_ptr<ShmChunk>> shm_chunk_list_;
};


#define SHM_NAME "/my_shm"
#define SHM_CAPACITY 4096
#define SHM_NUM 4096
#define SEM_NAME "/my_sem"
#define SEM_RUN 10000000

int run_writer() {
  SemLock sem_lock;

  if (sem_lock.Init(SEM_NAME, true)) {
    perror("run_writer sem.Init()");
    return -1;
  }

  for (int i=0; i<SEM_RUN; i++) {
    sem_lock.Lock();
    if (i == 0) {
      usleep(3500000);
    } else if (i%1000 == 0) {
      printf("%s %d entered lock \n", __func__, i);

      usleep(100000);
    }
    sem_lock.UnLock();
  }

  return 0;
}

int run_reader() {
  SemLock sem_lock;

  if (sem_lock.Init(SEM_NAME, false)) {
    perror("run_writer sem.Init()");
    return -1;
  }

  for (int i=0; i<SEM_RUN; i++) {
    sem_lock.Lock();
    if (i%1000 == 0) {
      printf("%s %d entered lock\n", __func__, i);
    }
    sem_lock.UnLock();
  }

  return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
      fprintf(stderr, "Usage: %s [writer|reader]\n", argv[0]);
      return 1;
    }

    if (strcmp(argv[1], "writer") == 0) {
      run_writer();
    } else if (strcmp(argv[1], "reader") == 0) {
      run_reader();
    } else {
      fprintf(stderr, "Unknown mode: %s\n", argv[1]);
      return 1;
    }

    return 0;
}

