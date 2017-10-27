#ifndef __HAL_PD_GCCB_HPP__
#define __HAL_PD_GCCB_HPP__

#include "nic/include/base.h"
#include "nic/include/capri_common.h"
#include "nic/utils/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

using hal::utils::ht_ctxt_t;

namespace hal {
namespace pd {

#define P4PD_HBM_GC_RNMDR_QID                   CAPRI_HBM_GC_RNMDR_QID
#define P4PD_HBM_GC_TNMDR_QID                   CAPRI_HBM_GC_TNMDR_QID

#define P4PD_HBM_GC_PER_PRODUCER_RING_SIZE      CAPRI_HBM_GC_PER_PRODUCER_RING_SIZE
#define P4PD_HBM_GC_PER_PRODUCER_RING_SHIFT     CAPRI_HBM_GC_PER_PRODUCER_RING_SHIFT

#define P4PD_HBM_GC_CB_ENTRY_SIZE               64

extern hal_ret_t p4pd_init_gc_cbs(void);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_GCCB_HPP__

