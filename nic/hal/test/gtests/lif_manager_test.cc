#include "nic/hal/src/lif_manager_base.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <errno.h>

static uint8_t fail_buf[64];  // Any I/O here will fail
static uint8_t pass_buf[512];
namespace hal {

class MockLIFManager : public LIFManagerBase {
 public:
  // Utility test function.
  LIFQState *get_qs(uint32_t lif_id) { return GetLIFQState(lif_id); }

  virtual int32_t GetPCOffset(
      const char *handle, const char *prog_name,
      const char *label, uint8_t *offset) {
    return 0;
  }

 protected:
  virtual int32_t InitLIFQStateImpl(LIFQState *qstate) {
    if (qstate->lif_id < 2048)
      return 0;
    return -EINVAL;
  }

  virtual int32_t ReadQStateImpl(
      uint64_t q_addr, uint8_t *buf, uint32_t q_size) {
    if (buf == fail_buf)
      return -EIO;
    return 0;
  }

  virtual int32_t WriteQStateImpl(
      uint64_t q_addr, const uint8_t *buf, uint32_t q_size) {
    if (buf == fail_buf)
      return -EIO;
    return 0;
  }
};

}  // namespace hal

TEST(LIFManagerTest, TestALL)
{
  hal::MockLIFManager lm;
  hal::LIFQStateParams params;

  bzero(&params, sizeof(params));
  ASSERT_TRUE(lm.InitLIFQState(0, &params) < 0);
  ASSERT_TRUE(lm.InitLIFQState(30000, &params) < 0);
  ASSERT_TRUE(lm.LIFRangeAlloc(1, 10) == 1);
  ASSERT_TRUE(lm.LIFRangeAlloc(-1, 1) == 0);
  ASSERT_TRUE(lm.InitLIFQState(0, &params) == 0);
  ASSERT_TRUE(lm.LIFRangeAlloc(2044, 10) < 0);
  ASSERT_TRUE(lm.LIFRangeAlloc(2044, 2) == 2044);

  params.type[1].size = 1;
  params.type[1].entries = 10;
  params.type[3].size = 2;
  params.type[3].entries = 10;
  ASSERT_TRUE(lm.InitLIFQState(2044, &params) == 0);

  hal::LIFQState *qstate = lm.get_qs(2044);
  ASSERT_TRUE(qstate != nullptr);
  ASSERT_TRUE(qstate->allocation_size == 0x30100);
  ASSERT_TRUE(qstate->type[0].hbm_offset == 0x0);
  ASSERT_TRUE(qstate->type[0].qsize == 0x20);
  ASSERT_TRUE(qstate->type[0].num_queues == 0x2);
  ASSERT_TRUE(qstate->type[1].hbm_offset == 0x40);
  ASSERT_TRUE(qstate->type[1].qsize == 0x40);
  ASSERT_TRUE(qstate->type[1].num_queues == 0x400);
  ASSERT_TRUE(qstate->type[2].hbm_offset == 0x10040);
  ASSERT_TRUE(qstate->type[2].qsize == 0x20);
  ASSERT_TRUE(qstate->type[2].num_queues == 0x2);
  ASSERT_TRUE(qstate->type[3].hbm_offset == 0x10080);
  ASSERT_TRUE(qstate->type[3].qsize == 0x80);
  ASSERT_TRUE(qstate->type[3].num_queues == 0x400);
  ASSERT_TRUE(qstate->type[4].hbm_offset == 0x30080);
  ASSERT_TRUE(qstate->type[4].qsize == 0x20);
  ASSERT_TRUE(qstate->type[4].num_queues == 0x1);
  ASSERT_TRUE(qstate->type[5].hbm_offset == 0x300a0);
  ASSERT_TRUE(qstate->type[5].qsize == 0x20);
  ASSERT_TRUE(qstate->type[5].num_queues == 0x1);
  ASSERT_TRUE(qstate->type[6].hbm_offset == 0x300c0);
  ASSERT_TRUE(qstate->type[6].qsize == 0x20);
  ASSERT_TRUE(qstate->type[6].num_queues == 0x1);
  ASSERT_TRUE(qstate->type[7].hbm_offset == 0x300e0);
  ASSERT_TRUE(qstate->type[7].qsize == 0x20);
  ASSERT_TRUE(qstate->type[7].num_queues == 0x1);

  ASSERT_TRUE(lm.ReadQState(3000, 0, 0, nullptr, 0) < 0);
  ASSERT_TRUE(lm.ReadQState(0, 0, 0, nullptr, 0) < 0);
  ASSERT_TRUE(lm.ReadQState(0, 0, 0, pass_buf, 0) < 0);
  ASSERT_TRUE(lm.WriteQState(3000, 0, 0, nullptr, 0) < 0);
  ASSERT_TRUE(lm.WriteQState(0, 0, 0, nullptr, 0) < 0);
  ASSERT_TRUE(lm.WriteQState(0, 0, 0, pass_buf, 0) < 0);
  ASSERT_TRUE(lm.WriteQState(2044, 0, 0, pass_buf, 32) == 0);
  ASSERT_TRUE(lm.WriteQState(2044, 0, 0, fail_buf, 32) < 0);
  ASSERT_TRUE(lm.ReadQState(2044, 0, 0, pass_buf, 64) == 0);
  ASSERT_TRUE(lm.ReadQState(2044, 0, 0, fail_buf, 64) < 0);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
