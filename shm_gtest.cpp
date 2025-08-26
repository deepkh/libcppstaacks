#include "ipc_helper.hpp"
#include "ipc_shm.hpp"

#include <gtest/gtest.h>

#include <unistd.h>      // fork()
#include <sys/wait.h>    // WIFEXITED, etc.

class ShmSeqTest : public ::testing::Test
{
  public:
  std::string SHM_NAME = "/my_shm";
  std::string SEM_NAME = "/my_sem"; 
  int SHM_CAPACITY = 64;
  int SHM_NUM = 16;
  int SEM_RUN = 10000;

  int run_reader(int pid);
  int run_writer(int pid);
};

int ShmSeqTest::run_writer(int pid)
{
  ipc::ShmQueue shm_queue;
  uint64_t *data;
  uint64_t sum = 0;
  if (shm_queue.Init(SHM_NAME, true, true, SHM_CAPACITY, SHM_NUM)) {
    printf("%s failed to shm_queue.Init\n", __func__);
    return -1;
  }
    
  usleep(1000000);

  for (uint64_t i=0; i<SEM_RUN; i++) {
    int j = i%SHM_NUM;

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
    usleep(50);
  }

  printf("[WRITER] TOTAL:%ld\n", sum);
  return 0;

}

int ShmSeqTest::run_reader(int pid)
{

  ipc::ShmQueue shm_queue;
  uint64_t data;
  int size;
  uint64_t sum = 0;

  usleep( 500000 );

  if (shm_queue.Init(SHM_NAME, false, false, SHM_CAPACITY, SHM_NUM)) {
    printf("%s failed to shm_queue.Init\n", __func__);
    return -1;
  }

  uint64_t seq_curr = 0;

  for (uint64_t i=0; i<SEM_RUN; i++) {
    int j = i%SHM_NUM;
    uint64_t seq = 0;

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

    seq_curr = seq;

  }
  printf("[READER] TOTAL:%ld\n", sum);

  return 0;
}

// Example test case for your shm_test project
TEST_F(ShmSeqTest, ReaderWriter) 
{
   int pid = fork();
   if ( 0 == pid ) {  // I am the child
      printf("HI FUCK IM CHILD\n");
      run_reader(0);
      exit(0);
   }
   
   // I am the parent
   printf("HI FUCK IM PARENT\n");
   run_writer(0);
   ASSERT_EQ( 0, ipc::helper::wait_for_child_fork( pid ) );  // wait for invalid pid
}



class ShmLargeDataTest : public ::testing::Test
{
public:
  std::string SHM_NAME = "/my_shm";
  std::string SEM_NAME = "/my_sem"; 
  int SHM_CAPACITY = 32000;
  int SHM_NUM = 4;
  int SEM_RUN = 10000;

  int run_reader(int pid);
  int run_writer(int pid);
};

int ShmLargeDataTest::run_writer(int pid)
{
  ipc::ShmQueue shm_queue;
  uint64_t *data;
  uint64_t sum = 0;
  if (shm_queue.Init(SHM_NAME, true, true, SHM_CAPACITY, SHM_NUM)) {
    printf("%s failed to shm_queue.Init\n", __func__);
    return -1;
  }
    
  usleep(1000000);

  for (uint64_t i=0; i<SEM_RUN; i++) {
    int j = i%SHM_NUM;

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
   
    usleep(50);
  }

  printf("[WRITER] TOTAL:%ld\n", sum);
  return 0;
}

int ShmLargeDataTest::run_reader(int pid)
{

  ipc::ShmQueue shm_queue;
  uint64_t data;
  int size;
  uint64_t sum = 0;

  usleep(500000);

  if (shm_queue.Init(SHM_NAME, false, false, SHM_CAPACITY, SHM_NUM)) {
    printf("%s failed to shm_queue.Init\n", __func__);
    return -1;
  }

  uint64_t seq_curr = 0;

  for (uint64_t i=0; i<SEM_RUN; i++) {
    int j = i%SHM_NUM;
    uint64_t seq = 0;

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

      // check md5 hex
      for (int j=0; j<16; j++) {
        //printf("%03d %02X %02X\n", j, buf[j], (uint8_t) md5_ret.second.data()[j]);
        if (buf[j] != (uint8_t) md5_ret.second.data()[j]) {
          break;
        }

        // last element are equal, so leave the loop
        if (buf[j] == (uint8_t) md5_ret.second.data()[j] && j == 15) {
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
      printf("CHECK SUM ERROR!\n");
      return -1;
    }

    seq_curr = seq;

  }
  printf("[READER] TOTAL:%ld\n", sum);
  return 0;
}

// Example test case for your shm_test project
TEST_F(ShmLargeDataTest, ReaderWriter) 
{
   int pid = fork();
   if ( 0 == pid ) {  // I am the child
      printf("HI FUCK IM CHILD\n");
      run_reader(0);
      exit(0);
   }
   
   // I am the parent
   printf("HI FUCK IM PARENT\n");
   run_writer(0);
   ASSERT_EQ( 0, ipc::helper::wait_for_child_fork( pid ) );  // wait for invalid pid
}


// main() entry for GoogleTest
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
