//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS MS Remote MAC to IP association
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_mac_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"

namespace pds_ms {

template<> sdk::lib::slab* slab_obj_t<mac_obj_t>::slab_ = nullptr;

constexpr int k_max_remote_macs = 1000;

void
mac_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-EVPN-MAC", slab_id,
                                      sizeof(mac_obj_t), 
                                      k_max_remote_macs,
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for EVPN Type2 MAC table");
    }
    mac_obj_t::set_slab(slabs_[slab_id].get());
}

} // End namespace
