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

class Shm {
public:
  Shm() {}

  ~Shm() {
    if (shm_fd_ > 0) {
      printf("[Shm] %s close\n", shm_name_.c_str());
      close(shm_fd_);
      if (creator_) {
        printf("[Shm] %s unlink\n", shm_name_.c_str());
        sem_unlink(shm_name_.c_str());
      }
    }
  }

  int Init(std::string shm_name, bool creator, int shm_size) {
    shm_name_ = shm_name;
    creator_ = creator;
    shm_size_ = shm_size;

    if (creator_) {
      printf("[Shm] %s create %d\n", shm_name_.c_str(), shm_size);
      shm_fd_ = shm_open(shm_name_.c_str(), O_CREAT | O_RDWR, 0666);
    } else {
      printf("[Shm] %s open\n", shm_name_.c_str());
      shm_fd_ = shm_open(shm_name_.c_str(), O_RDWR, 0666);
    }

    if (shm_fd_ == -1) {
      printf("error 1\n");
      return -1;
    }
      
    if (creator_ && ftruncate(shm_fd_, shm_size) == -1) {
      printf("error 2\n");
      return -2;
    }

    if (creator_) {
      shm_ptr_ = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    } else {
      shm_ptr_ = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    }

    if (shm_ptr_ == MAP_FAILED) {
      printf("error 3\n");
      return -3;
    }

    return 0;
  }
  
  void *Data() {
    return shm_ptr_;
  }

  int Size() {
    return shm_size_;
  }
private:
  bool creator_ = false;
  std::string shm_name_;
  int shm_fd_ = -1;
  int shm_size_ = 0;
  void *shm_ptr_ = nullptr;
};

class Sem {
public:
  Sem() {}

  ~Sem() {
    if (sem_) {
      printf("[Sem] %s close\n", sem_name_.c_str());
      sem_close(sem_);
      if (creator_) {
        printf("[Sem] %s unlink\n", sem_name_.c_str());
        sem_unlink(sem_name_.c_str());
      }
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
        printf("sem error 1\n");
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
        printf("Semaphore value: %d\n", value);  // Should print 3 initially
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
  SemLock() {};
  ~SemLock() {};

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

struct ShmSlot {
  void *data;
  uint32_t size;
  uint32_t is_block;
};

#define SHM_NAME "/tmp/my_shm"
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

