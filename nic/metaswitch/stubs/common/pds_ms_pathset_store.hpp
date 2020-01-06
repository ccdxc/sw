//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub Overlay Pathset store for EVPN Type 5 routes
//---------------------------------------------------------------

#ifndef __PDS_MS_PATHSET_STORE_HPP__
#define __PDS_MS_PATHSET_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ecmp_idx_guard.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_object_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/lib/slab/slab.hpp"

namespace pds_ms {

// ECMP Pathset of VXLAN Tunnels used by Overlay Type 5 routes
class pathset_obj_t : public slab_obj_t<pathset_obj_t>,
                      public base_obj_t {
public:
    struct properties_t {
        ms_ps_id_t                ms_ps_id;
        properties_t(ms_ps_id_t ps_id) : ms_ps_id(ps_id) {}; 
    };
    // PDS HAL Overlay ECMP table entrie are created from both the
    // MS VXLAN Tunnel(Type2) and MS Pathset(Type5) HAL stub objects
    // requiring index mgmt.
    // Index Guard provides automatic alloc/free of Overlay ECMP index
    // in the ctor/dtor.
    // Needs to be shared_ptr to avoid premature freeing of index
    // in destructor as part of update
    std::shared_ptr<ecmp_idx_guard_t>   hal_oecmp_idx_guard; 

    pathset_obj_t(ms_ps_id_t ps_id);
    properties_t& properties(void) {return prop_;}

    ms_ps_id_t key(void) const {return prop_.ms_ps_id;}
    void update_store(state_t* state, bool op_delete) override;
    void print_debug_str(void) override {};

private:  
    properties_t prop_;
};

class pathset_store_t : public obj_store_t <ms_ps_id_t, pathset_obj_t> {
};

void pathset_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);

}

#endif

