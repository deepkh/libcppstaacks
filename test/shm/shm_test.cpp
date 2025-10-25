
#include <cppstuff/shm/shm.hpp>
#include <cppstuff/helper/ipc_helper.hpp>


#define SHM_NAME "/my_shm"
//#define SHM_CAPACITY 64
#define SHM_CAPACITY 16000
#define SHM_NUM 16
#define SEM_NAME "/my_sem"
#define SEM_RUN 10000

int run_writer() {
#if 0
  ipc::SemLock sem_lock;

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
  ipc::ShmQueue shm_queue;
  uint64_t *data;
  uint64_t sum = 0;
  if (shm_queue.Init(SHM_NAME, true, true, SHM_CAPACITY, SHM_NUM)) {
    printf("%s failed to shm_queue.Init\n", __func__);
    return -1;
  }
    
  usleep(5000000);

  for (uint64_t i=0; i<SEM_RUN; i++) {
    int j = i%SHM_NUM;
#if 0
    auto chk = shm_queue.At(j);
    chk->Lock();
    chk->Data().Seq() = i;
    chk->Data().Size() = sizeof(i);

    data = reinterpret_cast<uint64_t*>(shmGuard.Data().Addr());
    *data = i;
    sum += *data;

    printf("[WRITER] j:%8d seq:%16ld data:%16ld size:%d sum:%ld\n"
        , j, chk->Data().Head()->seq
        , *data
        , chk->Data().Size()
        , sum);
    chk->UnLock();

#else
#if 0
    ipc::ShmQueue::ShmChunkGuard shmGuard = shm_queue.Enqueue(j);
    shmGuard.Data().Seq() = i;
    shmGuard.Data().Size() = sizeof(i);

    data = reinterpret_cast<uint64_t*>(shmGuard.Data().Addr());
    *data = i;
    shmGuard.Dequeue();
    
    sum += *data;
    printf("[WRITER] j:%8d seq:%16ld data:%16ld size:%d sum:%ld\n"
        , j, shmGuard.Data().Head()->seq
        , *data
        , shmGuard.Data().Size()
        , sum);
#else
    ipc::ShmQueue::ShmChunkGuard shmGuard = shm_queue.Enqueue(j);
    auto buf = shmGuard.Data().Addr();
    int size = SHM_CAPACITY;
    shmGuard.Data().Seq() = i;
    shmGuard.Data().Size() = size;

    // gen random data into buf
    random_data_generator::gen(buf, size);

    // calc md5 for the buf
    auto md5_ret = random_data_generator::md5(buf + 16, size - 16);
    std::span<uint8_t> view(buf + 16, size - 16);
    for (auto &b: view) {
      sum = (sum + b) % 256;
    }

    // fill md5 hex
    for (int j=0; j<16; j++) {
      buf[j] =  md5_ret.second.data()[j];
    }

    shmGuard.Dequeue();

    printf("[WRITER] j:%8d seq:%16ld size:%d md5:%s sum:%ld \n"
        , j
        , i
        , size
        , md5_ret.first.c_str()
        , sum);

#endif
#endif
    usleep(50);
    //usleep(1000000);
  }

  printf("TOTAL:%ld\n", sum);
#endif
  return 0;
}

int run_reader() {
#if 0
  ipc::SemLock sem_lock;

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
  ipc::ShmQueue shm_queue;
  uint64_t data;
  int size;
  uint64_t sum = 0;
  if (shm_queue.Init(SHM_NAME, false, false, SHM_CAPACITY, SHM_NUM)) {
    printf("%s failed to shm_queue.Init\n", __func__);
    return -1;
  }

  uint64_t seq_curr = 0;

  for (uint64_t i=0; i<SEM_RUN; i++) {
    int j = i%SHM_NUM;
    uint64_t seq = 0;

#if 0
    auto chk = shm_queue.At(j);

    do {
      chk->Lock();
      seq = chk->Data().Seq();
      size = chk->Data().Size();
      data = *(reinterpret_cast<uint64_t*>(chk->Data().Addr()));
      if (seq_curr <= seq && size > 0) {
        sum += data;
        chk->UnLock();
        break;
      }
      chk->UnLock();
      usleep(1);
    } while(true);
#else
#if 0

    do {
      ipc::ShmQueue::ShmChunkGuard shmGuard = shm_queue.Enqueue(j);
      seq = shmGuard.Data().Seq();
      size = shmGuard.Data().Size();
      data = *(reinterpret_cast<uint64_t*>(shmGuard.Data().Addr()));
      shmGuard.Dequeue();
      if (seq_curr <= seq && size > 0) {
        sum += data;
        break;
      }
      usleep(1);
    } while(true);

    printf("[READER] j:%8d seq:%16ld data:%16ld size:%d sum:%ld\n"
        , j
        , seq
        , data
        , size
        , sum);


#else
    std::pair<std::string, std::array<char, MD5_DIGEST_LENGTH>> md5_ret;
    bool is_checksum_equal = false;
    do {
      ipc::ShmQueue::ShmChunkGuard shmGuard = shm_queue.Enqueue(j);
      seq = shmGuard.Data().Seq();
      size = shmGuard.Data().Size();
      auto buf = shmGuard.Data().Addr();
      
      // check if the slot is not reach out
      if (!(seq_curr <= seq && size > 0)) {
        shmGuard.Dequeue();
        usleep(1);
        continue;
      }

      // calc md5 for the buf
      md5_ret = random_data_generator::md5(buf + 16, size - 16);
      std::span<uint8_t> view(buf + 16, size - 16);
      for (auto &b: view) {
        sum = (sum + b) % 256;
      }

      // fill md5 hex
      for (int j=0; j<16; j++) {
        if (buf[j] != md5_ret.second.data()[j]) {
          break;
        }

        // last element are equal, so leave the loop
        if (buf[j] == md5_ret.second.data()[j] && j == 15) {
          is_checksum_equal = true;
          break;
        }
      }

      shmGuard.Dequeue();
      break;
    } while(true);

    printf("[READER] j:%8d seq:%16ld size:%d md5:%s sum:%ld eq:%d\n"
    , j
    , seq
    , size
    , md5_ret.first.c_str()
    , sum
    , is_checksum_equal);

    if (!is_checksum_equal) {
      return -1;
    }

#endif
#endif
    seq_curr = seq;

  }
  printf("TOTAL:%ld\n", sum);


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

