//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub Remote VXLAN Tunnel Endpoint store
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_tep_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"

namespace pds_ms {

template<> sdk::lib::slab* slab_obj_t<tep_obj_t>::slab_ = nullptr;

void
tep_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-TEP", slab_id, 
                                      sizeof(tep_obj_t), 
                                      100,   // 1/10 of PDS_MAX_TEP
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for TEP");
    }
    tep_obj_t::set_slab(slabs_[slab_id].get());
}

const tep_dmac_t* tep_obj_t::dmac_info(const mac_addr_wr_t& mac) const
{
    auto it = dmacs_.find(mac);
    if (it == dmacs_.end()) {
        return nullptr;
    }
    return &(it->second);
}

bool tep_obj_t::dmac_info(const mac_addr_wr_t& mac, tep_dmac_t* out_dmac_entry) const
{
    auto it = dmacs_.find(mac);
    if (it == dmacs_.end()) {
        return false;
    }
    *out_dmac_entry = it->second;
    return true;
}

void tep_obj_t::update_store (state_t* state, bool op_delete)
{
    if (!op_delete) {
        state->tep_store().add_upd(this->key(), this);
        SDK_TRACE_DEBUG ("Add TEP %s to store: hal_tep_idx_ %ld, "
                         "hal_uecmp_idx_ %ld, hal_oecmp_idx_ %ld", 
                         ipaddr2str(&prop_.tep_ip), prop_.hal_tep_idx, 
                         prop_.hal_uecmp_idx, prop_.hal_oecmp_idx);
    } else { 
        state->tep_store().erase(this->key());
        SDK_TRACE_DEBUG ("Delete TEP %s from store", 
                         ipaddr2str(&prop_.tep_ip));
    }
}

} // End namespace
