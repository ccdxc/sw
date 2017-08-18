#include "lif_manager_base.hpp"
#include <stdio.h>
#include <assert.h>
#include <errno.h>

static uint8_t fail_buf[64];  // Any I/O here will fail
static uint8_t pass_buf[512];
namespace hal {

class MockLIFManager : public LIFManagerBase {
 public:
  // Utility test function.
  LIFQState *get_qs(uint32_t lif_id) { return GetLIFQState(lif_id); }

  virtual int32_t GetPCOffset(
      const char *handle, char *prog_name, char *label, uint8_t *offset) {
    return 0;
  }

 protected:
  virtual int32_t InitLIFQStateImpl(LIFQState *qstate) {
    if (qstate->lif_id < 2048)
      return 0;
    return -EINVAL;
  }

  virtual int32_t ReadQStateImpl(
      uint32_t q_addr, uint8_t *buf, uint32_t q_size) {
    if (buf == fail_buf)
      return -EIO;
    return 0;
  }

  virtual int32_t WriteQStateImpl(
      uint32_t q_addr, uint8_t *buf, uint32_t q_size) {
    if (buf == fail_buf)
      return -EIO;
    return 0;
  }
};

}

int
main()
{
  hal::MockLIFManager lm;
  hal::LIFQStateParams params;

  bzero(&params, sizeof(params));
  assert(lm.InitLIFQState(0, &params) < 0);
  assert(lm.InitLIFQState(30000, &params) < 0);
  assert(lm.LIFRangeAlloc(1, 10) == 1);
  assert(lm.LIFRangeAlloc(-1, 1) == 0);
  assert(lm.InitLIFQState(0, &params) == 0);
  assert(lm.LIFRangeAlloc(2044, 10) < 0);
  assert(lm.LIFRangeAlloc(2044, 2) == 2044);

  params.type[1].size = 1;
  params.type[1].entries = 10;
  params.type[3].size = 2;
  params.type[3].entries = 10;
  assert(lm.InitLIFQState(2044, &params) == 0);

  hal::LIFQState *qstate = lm.get_qs(2044);
  assert(qstate != nullptr);
  assert(qstate->allocation_size == 0x30100);
  assert(qstate->type[0].hbm_offset == 0x0);
  assert(qstate->type[0].qsize == 0x20);
  assert(qstate->type[0].num_queues == 0x2);
  assert(qstate->type[1].hbm_offset == 0x40);
  assert(qstate->type[1].qsize == 0x40);
  assert(qstate->type[1].num_queues == 0x400);
  assert(qstate->type[2].hbm_offset == 0x10040);
  assert(qstate->type[2].qsize == 0x20);
  assert(qstate->type[2].num_queues == 0x2);
  assert(qstate->type[3].hbm_offset == 0x10080);
  assert(qstate->type[3].qsize == 0x80);
  assert(qstate->type[3].num_queues == 0x400);
  assert(qstate->type[4].hbm_offset == 0x30080);
  assert(qstate->type[4].qsize == 0x20);
  assert(qstate->type[4].num_queues == 0x1);
  assert(qstate->type[5].hbm_offset == 0x300a0);
  assert(qstate->type[5].qsize == 0x20);
  assert(qstate->type[5].num_queues == 0x1);
  assert(qstate->type[6].hbm_offset == 0x300c0);
  assert(qstate->type[6].qsize == 0x20);
  assert(qstate->type[6].num_queues == 0x1);
  assert(qstate->type[7].hbm_offset == 0x300e0);
  assert(qstate->type[7].qsize == 0x20);
  assert(qstate->type[7].num_queues == 0x1);

  assert(lm.ReadQState(3000, 0, 0, nullptr, 0) < 0);
  assert(lm.ReadQState(0, 0, 0, nullptr, 0) < 0);
  assert(lm.ReadQState(0, 0, 0, pass_buf, 0) < 0);
  assert(lm.WriteQState(3000, 0, 0, nullptr, 0) < 0);
  assert(lm.WriteQState(0, 0, 0, nullptr, 0) < 0);
  assert(lm.WriteQState(0, 0, 0, pass_buf, 0) < 0);
  assert(lm.WriteQState(2044, 0, 0, pass_buf, 32) == 0);
  assert(lm.WriteQState(2044, 0, 0, fail_buf, 32) < 0);
  assert(lm.ReadQState(2044, 0, 0, pass_buf, 64) == 0);
  assert(lm.ReadQState(2044, 0, 0, fail_buf, 64) < 0);

  printf("LIF manager tests passed.\n");
  return 0;
}
