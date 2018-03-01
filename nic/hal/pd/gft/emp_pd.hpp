// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_EMP_PD_HPP__
#define __HAL_EMP_PD_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/globalpd/gpd_utils.hpp"

namespace hal {
namespace pd {

#define RX_KEY_TABLE_IDS(ENTRY)                             \
    ENTRY(RX_KEY_TABLE_ID1,    0, "RX_KEY_TABLE_ID1")       \
    ENTRY(RX_KEY_TABLE_ID2,    1, "RX_KEY_TABLE_ID2")       \
    ENTRY(RX_KEY_TABLE_ID3,    2, "RX_KEY_TABLE_ID3")       \
    ENTRY(RX_KEY_TABLE_ID4,    3, "RX_KEY_TABLE_ID4")

    DEFINE_ENUM(rx_table_id_t, RX_KEY_TABLE_IDS)
#undef RX_KEY_TABLE_IDS


struct pd_gft_emp_s {

    uint32_t    rx_key1_idx;
    uint32_t    rx_key2_idx;
    uint32_t    rx_key3_idx;
    uint32_t    rx_key4_idx;

    uint32_t    tx_key1_idx;

    // pi ptr
    void        *pi_emp;
} __PACK__;


}   // namespace pd
}   // namespace hal

#endif    // __HAL_EMP_PD_HPP__

