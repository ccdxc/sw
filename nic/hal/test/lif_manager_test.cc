#include "lif_manager_base.hpp"
#include <stdio.h>
#include <assert.h>
#include <errno.h>

namespace hal {

class MockLIFManager : public LIFManagerBase {
 protected:
  virtual int32_t InitLIFToQstateImpl(LIFToQstateParams *params,
                                      uint64_t *ret_handle) {
    *ret_handle = 0;
    if (params->lif_id < 2048)
      return 0;
    return -EINVAL;
  }
};

}

int
main()
{
  hal::MockLIFManager lm;

  assert(lm.GetLIFToQstateParams(0) == nullptr);
  assert(lm.GetLIFToQstateParams(30000) == nullptr);
  assert(lm.LIFRangeAlloc(1, 10) == 1);
  assert(lm.LIFRangeAlloc(-1, 1) == 0);
  assert(lm.GetLIFToQstateParams(0) != nullptr);
  assert(lm.LIFRangeAlloc(2044, 10) < 0);
  assert(lm.LIFRangeAlloc(2044, 2) == 2044);
  auto params = lm.GetLIFToQstateParams(2045);
  assert(params != nullptr);
  assert(params->lif_id == 2045);
  assert(lm.InitLIFToQstate(params) == 0);
  printf("LIF manager tests passed.\n");
  return 0;
}
