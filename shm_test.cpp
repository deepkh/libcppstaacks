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
      //printf("[Sem] %s close\n", sem_name_.c_str());
      sem_close(sem_);
      if (creator_) {
        //printf("[Sem] %s unlink\n", sem_name_.c_str());
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

  int Init(const std::string &sem_name, bool creator) {
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
      //printf("[ShmAlloc] %s close\n", shm_name_.c_str());
      close(fd_);
      fd_ = -1;

      if (creator_) {
        //printf("[ShmAlloc] %s unlink\n", shm_name_.c_str());
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
      //printf("[ShmAlloc] %s create %d\n", shm_name_.c_str(), size_);
      fd_ = shm_open(shm_name_.c_str(), O_CREAT | O_RDWR, 0666);
    } else {
      //printf("[ShmAlloc] %s open %d\n", shm_name_.c_str(), size_);
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

    if ((mapped_addr = mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0)) == MAP_FAILED) {
      printf("[ShmAlloc] error 3\n");
      return -3;
    }

    mapped_addr_ = static_cast<uint8_t *>(mapped_addr);

    if (creator_ && reset) {
      printf("[ShmAlloc] reset %d to zero\n", size_);
      memset(mapped_addr_, 0 , size_);
    }

    printf("[ShmAlloc] fd:%d mapped:%p size:%d\n", fd_, mapped_addr_, size_);
    return 0;
  }
  
  uint8_t *MappedAddr() {
    return mapped_addr_;
  }

  int Size() {
    return size_;
  }

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

  ShmDataHead *Head() {
    return head_;
  }

  uint8_t *Addr() {
    return addr_;
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
  uint8_t *addr_ = nullptr;
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
    printf("[ShmChunk] %s %p capacity:%d size:%d seq:%lu\n"
      , Name().c_str()
      , shm_data_.Addr()
      , shm_data_.Capacity()
      , shm_data_.Size()
      , shm_data_.Seq());

    return 0;
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
  ShmQueue() {}
  ~ShmQueue() {}

  void Close() {
    shm_alloc_.Close();
    shm_chunk_list_.clear();
  }

  int Init(const std::string &shm_name, bool creator, bool reset, int chunk_capacity, int chunk_num) {
    const int shm_size = sizeof(ShmQueueHead) + (chunk_capacity + sizeof(ShmDataHead)) * chunk_num;
    printf("%d %d %d %d\n"
      , sizeof(ShmQueueHead), chunk_capacity
      , sizeof(ShmDataHead), chunk_num);
    if (shm_alloc_.Init(shm_name, creator, reset, shm_size)) {
      printf("[ShmQueue] failed to alloc %d\n", shm_size);
      return -1;
    }

    uint8_t *mapped_ptr = shm_alloc_.MappedAddr();
    head_ = static_cast<ShmQueueHead*>((void*)mapped_ptr);
    mapped_ptr += sizeof(ShmQueueHead);

    if (creator && reset) {
      head_->Reset();
    }

    for (int i=0; i<chunk_num; i++) {
      const std::string chunk_name = shm_name +"-chunk-" + std::to_string(i);
      std::shared_ptr<ShmChunk> shm_chunk = std::make_shared<ShmChunk>();
      if (shm_chunk->Init(chunk_name, creator, mapped_ptr, chunk_capacity)) {
        printf("[ShmQueue] failed to shm_chunk->Init %s\n", chunk_name.c_str());
        return -1;
      }

      shm_chunk_list_.emplace_back(shm_chunk);
      mapped_ptr += chunk_capacity+sizeof(ShmDataHead);
    }

    return 0;
  }

  ShmQueueHead *Head() {
    return head_;
  }

  std::shared_ptr<ShmChunk> At(int i) {
    return shm_chunk_list_.at(i);
  }

  int Size() {
    return shm_chunk_list_.size();
  }

private:
  ShmQueueHead *head_;
  ShmAlloc shm_alloc_;
  std::vector<std::shared_ptr<ShmChunk>> shm_chunk_list_;
};


#define SHM_NAME "/my_shm"
#define SHM_CAPACITY 64
#define SHM_NUM 16
#define SEM_NAME "/my_sem"
#define SEM_RUN 10000000

int run_writer() {
#if 0
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
#else
  ShmQueue shm_queue;
  if (shm_queue.Init(SHM_NAME, true, true, SHM_CAPACITY, SHM_NUM)) {
    printf("%s failed to shm_queue.Init\n", __func__);
    return -1;
  }

  for (int i=0; i<SEM_RUN; i++) {
    int j = i%SHM_NUM;
    auto chk = shm_queue.At(j);
    chk->Lock();
    chk->Data().Head()->seq = i;
    printf("[WRITER] j:%8d seq:%16ld \n", j, chk->Data().Head()->seq);
    chk->UnLock();
    usleep(1000000);
  }
#endif
  return 0;
}

int run_reader() {
#if 0
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
#else
  ShmQueue shm_queue;
  if (shm_queue.Init(SHM_NAME, false, false, SHM_CAPACITY, SHM_NUM)) {
    printf("%s failed to shm_queue.Init\n", __func__);
    return -1;
  }

  uint64_t seq_curr = 0;

  for (int i=0; i<SEM_RUN; i++) {
    int j = i%SHM_NUM;
    uint64_t seq = 0;
    auto chk = shm_queue.At(j);

    do {
      chk->Lock();
      seq = chk->Data().Head()->seq;
      chk->UnLock();
      if (seq_curr <= seq) {
        break;
      }
      usleep(1);
    } while(true);

    seq_curr = seq;

    printf("[READER] j:%8d seq:%16ld \n", j, seq);
    usleep(500000);
  }


#endif

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

