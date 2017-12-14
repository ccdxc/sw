#ifndef __BARCO_RINGS_HPP__
#define __BARCO_RINGS_HPP__
#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/barco_rings.pb.h"
#include "nic/gen/proto/hal/types.pb.h"

using std::string;
using barcoRings::GetOpaqueTagAddrRequest;
using barcoRings::GetOpaqueTagAddrResponse;
using barcoRings::GetOpaqueTagAddrRequestMsg;
using barcoRings::GetOpaqueTagAddrResponseMsg;
using barcoRings::BarcoGetReqDescrEntryRequest;
using barcoRings::BarcoGetReqDescrEntryRequestMsg;
using barcoRings::BarcoGetReqDescrEntryResponse;
using barcoRings::BarcoGetReqDescrEntryResponseMsg;
using barcoRings::BarcoGetRingMetaRequest;
using barcoRings::BarcoGetRingMetaRequestMsg;
using barcoRings::BarcoGetRingMetaResponse;
using barcoRings::BarcoGetRingMetaResponseMsg;

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

hal_ret_t   GetOpaqueTagAddr(const GetOpaqueTagAddrRequest& request,
    GetOpaqueTagAddrResponse *response);

hal_ret_t   BarcoGetReqDescrEntry(const BarcoGetReqDescrEntryRequest& request,
				  BarcoGetReqDescrEntryResponse *response);

hal_ret_t   BarcoGetRingMeta(const BarcoGetRingMetaRequest& request,
			     BarcoGetRingMetaResponse *response);

}    // namespace hal
#endif  /* __BARCO_RINGS_HPP__*/

