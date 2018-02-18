// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_EFE_PD_HPP__
#define __HAL_EFE_PD_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/globalpd/gpd_utils.hpp"

namespace hal {
namespace pd {

typedef struct gft_flow_hash_data_s {
     uint8_t     export_en:1;
     uint16_t    flow_index;
     uint16_t    policer_index;
 } __PACK__ gft_flow_hash_data_t;

struct pd_gft_efe_s {

    uint32_t	flow_table_idx;
	uint32_t	flow_idx;
    uint32_t    policer_idx;

    // pi ptr
    void        *pi_efe;
} __PACK__;


}   // namespace pd
}   // namespace hal

#endif    // __HAL_EFE_PD_HPP__

