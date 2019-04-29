//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef _NVME_HPP_
#define _NVME_HPP_

#include "nic/include/base.hpp"
#include <memory>
#include <map>
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "gen/proto/nvme.pb.h"
#include "nic/include/hal.hpp"
#include "lib/bm_allocator/bm_allocator.hpp"

using nvme::NvmeSqSpec;
using nvme::NvmeSqRequestMsg;
using nvme::NvmeSqResponse;
using nvme::NvmeSqResponseMsg;
using nvme::NvmeCqSpec;
using nvme::NvmeCqRequestMsg;
using nvme::NvmeCqResponse;
using nvme::NvmeCqResponseMsg;
using nvme::NvmeNsSpec;
using nvme::NvmeNsRequestMsg;
using nvme::NvmeNsResponse;
using nvme::NvmeNsResponseMsg;
using nvme::NvmeEnableRequest;
using nvme::NvmeEnableResponse;
using nvme::NvmeEnableRequestMsg;
using nvme::NvmeEnableResponseMsg;

namespace hal {

/* Extern functions called by lif plugin */
extern hal_ret_t nvme_lif_init(intf::LifSpec& spec, uint32_t lif_id);
extern uint64_t nvme_lif_pt_base_addr(uint32_t lif_id);

/* Services exposed by NVME HAL */
hal_ret_t nvme_sq_create (NvmeSqSpec& spec, NvmeSqResponse *rsp);
hal_ret_t nvme_cq_create (NvmeCqSpec& spec, NvmeCqResponse *rsp);
hal_ret_t nvme_ns_create (NvmeNsSpec& spec, NvmeNsResponse *rsp);
hal_ret_t nvme_enable (NvmeEnableRequest& spec, NvmeEnableResponse *rsp);


class NVMEManager {
 public:
  NVMEManager();
  uint64_t HbmAlloc(uint32_t size);
  uint64_t hbm_base_;

 protected:

 private:
  std::unique_ptr<sdk::lib::BMAllocator> hbm_allocator_;

  // Track allocation size which are needed when we
  // free memory.
  std::map<uint64_t, uint64_t> allocation_sizes_;
};

extern NVMEManager *nvme_manager();

} // namepsace hal

#endif //NVME_HPP

