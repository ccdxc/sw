#include "nic/utils/host_mem/host_mem.hpp"
#include "gtest/gtest.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <assert.h>
#include <memory>

using namespace utils;

static const size_t kShmAvlSize = kShmSize / 2;
class HostMemTest : public ::testing::Test {
 protected:
    virtual void SetUp() {
        // Remove any stale segments.
        shmid_ = shmget(HostMemHandle(), kShmSize, 0666);
        if (shmid_ >= 0)
            shmctl(shmid_, IPC_RMID, NULL);

        shmid_ = shmget(HostMemHandle(), kShmSize, IPC_CREAT | 0666);
        assert(shmid_ >= 0);
    }

    virtual void TearDown() {
        shmctl(shmid_, IPC_RMID, NULL);
    }

 private:
    int shmid_;
};

TEST_F(HostMemTest, TestCreate) {
  std::unique_ptr<HostMem> mem(HostMem::New());
  ASSERT_NE(nullptr, mem.get());
}

TEST_F(HostMemTest, TestAllocFree) {
  std::unique_ptr<HostMem> mem(HostMem::New());
  ASSERT_NE(nullptr, mem.get());
  void *ptr = mem->Alloc(kShmAvlSize - kAllocUnit);
  ASSERT_TRUE(ptr != nullptr);
  void *ptr2 = mem->Alloc(kShmAvlSize - kAllocUnit);
  ASSERT_TRUE(ptr2 == nullptr);
  mem->Free(ptr);
  ptr2 = mem->Alloc(kShmAvlSize - kAllocUnit);
  ASSERT_TRUE(ptr2 != nullptr);
}

TEST_F(HostMemTest, TestSharing) {
  std::unique_ptr<HostMem> mem(HostMem::New());
  ASSERT_NE(nullptr, mem.get());
  uint8_t *ptr = (uint8_t *)mem->Alloc(1);
  ASSERT_TRUE(ptr != nullptr);
  *ptr = 0x55;
  pid_t pid = fork();
  if (pid == 0) {
    ASSERT_TRUE(*ptr == 0x55);
    *ptr = 0xAA;
    exit(0);
  }
  waitpid(0, 0, 0);
  ASSERT_EQ(*ptr, 0xAA);
}

bool check_align(void *ptr, uint64_t align) {
  uint64_t v = (uint64_t)(ptr);
  if (!v)
    return false;
  if (v & (align - 1))
    return false;
  return true;
}

TEST_F(HostMemTest, TestAlign) {
  std::unique_ptr<HostMem> mem(HostMem::New());
  ASSERT_NE(nullptr, mem.get());
  ASSERT_TRUE(check_align(mem->Alloc(1, 32), 32));
  ASSERT_TRUE(check_align(mem->Alloc(1, 4096), 4096));
  ASSERT_TRUE(check_align(mem->Alloc(1, 64), 64));
  ASSERT_TRUE(check_align(mem->Alloc(1, 1024), 1024));
  ASSERT_TRUE(check_align(mem->Alloc(kShmAvlSize - 8192, 4096), 4096));
  ASSERT_TRUE(check_align(mem->Alloc(1, 64), 64));
  ASSERT_EQ(mem->Alloc(1, 4096), nullptr);
}
  
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
