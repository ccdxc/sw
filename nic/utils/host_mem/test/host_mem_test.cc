#include "host_mem.hpp"
#include "gtest/gtest.h"

#include <unistd.h>
#include <memory>

using namespace utils;

TEST(HostMemTest, TestCreate) {
  std::unique_ptr<HostMem> mem(HostMem::New());
  ASSERT_NE(nullptr, mem.get());
}

TEST(HostMemTest, TestAllocFree) {
  std::unique_ptr<HostMem> mem(HostMem::New());
  ASSERT_NE(nullptr, mem.get());
  void *ptr = mem->Alloc(kShmSize - kAllocUnit);
  ASSERT_TRUE(ptr != nullptr);
  void *ptr2 = mem->Alloc(kShmSize - kAllocUnit);
  ASSERT_TRUE(ptr2 == nullptr);
  mem->Free(ptr);
  ptr2 = mem->Alloc(kShmSize - kAllocUnit);
  ASSERT_TRUE(ptr2 != nullptr);
}

TEST(HostMemTest, TestSharing) {
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
  
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

