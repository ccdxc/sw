//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS Metaswitch stub Subnet Spec store
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_subnet_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"

namespace pds_ms {
template<> sdk::lib::slab* slab_obj_t<subnet_obj_t>::slab_ = nullptr;

void
subnet_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDSA-SUBNET", slab_id, 
                                      sizeof(subnet_obj_t), 
                                      8,               // PDS_MAX_SUBNET/8 
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for Subnet");
    }
    subnet_obj_t::set_slab(slabs_[slab_id].get());

}

} // End namespace

