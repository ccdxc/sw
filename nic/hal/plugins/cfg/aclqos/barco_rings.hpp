//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __BARCO_RINGS_HPP__
#define __BARCO_RINGS_HPP__
#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "gen/proto/internal.pb.h"
#include "gen/proto/types.pb.h"

using std::string;
using internal::GetOpaqueTagAddrRequest;
using internal::GetOpaqueTagAddrResponse;
using internal::GetOpaqueTagAddrRequestMsg;
using internal::GetOpaqueTagAddrResponseMsg;
using internal::BarcoGetReqDescrEntryRequest;
using internal::BarcoGetReqDescrEntryRequestMsg;
using internal::BarcoGetReqDescrEntryResponse;
using internal::BarcoGetReqDescrEntryResponseMsg;
using internal::BarcoGetRingMetaRequest;
using internal::BarcoGetRingMetaRequestMsg;
using internal::BarcoGetRingMetaResponse;
using internal::BarcoGetRingMetaResponseMsg;

namespace hal {

typedef struct barco_symm_descr_s {
    uint64_t                ilist_addr;
    uint64_t                olist_addr;
    uint32_t                command;
    uint32_t                key_desc_index;
    uint64_t                iv_addr;
    uint64_t                status_addr;
    uint64_t                doorbell_addr;
    uint64_t                doorbell_data;
    uint32_t                salt;
    uint64_t                explicit_iv;
    uint32_t                barco_status;
    uint32_t                header_size;
    uint32_t                second_key_desc_index;
} barco_symm_descr_t;

typedef struct barco_asym_descr_s {
    uint64_t                ilist_addr;
    uint64_t                olist_addr;
    uint32_t                key_desc_index;
    uint64_t                status_addr;
    uint32_t                opaque_tag_value;
    uint32_t                opaque_tag_wr_en;
    uint32_t                flag_a;
    uint32_t                flag_b;
    uint32_t                barco_status;
} barco_asym_descr_t;


}    // namespace hal
#endif  /* __BARCO_RINGS_HPP__*/

