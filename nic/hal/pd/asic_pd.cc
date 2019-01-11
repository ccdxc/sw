// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/hal.hpp"
#include "nic/hal/pd/asic_pd.hpp"
#include "nic/sdk/lib/pal/pal.hpp"

namespace hal {
namespace pd {

//------------------------------------------------------------------------------
// public API for saving cpu packet
//------------------------------------------------------------------------------
sdk_ret_t
asic_step_cpu_pkt (const uint8_t* pkt, size_t pkt_len)
{
    pal_ret_t prc = sdk::lib::pal_step_cpu_pkt(pkt, pkt_len);
    return IS_PAL_API_SUCCESS(prc) ? SDK_RET_OK : SDK_RET_ERR;
}

}    // namespace pd
}    // namespace hal
