#ifndef __HAL_PD_GCCB_HPP__
#define __HAL_PD_GCCB_HPP__

#include "nic/include/base.h"
#include "nic/include/capri_common.h"
#include "sdk/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

extern hal_ret_t p4pd_init_gc_cbs(void);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_GCCB_HPP__

