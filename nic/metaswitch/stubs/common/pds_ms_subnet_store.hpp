//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS Metaswitch stub Subnet Spec store used by Mgmt
//---------------------------------------------------------------

#ifndef __PDS_MS_SUBNET_STORE_HPP__
#define __PDS_MS_SUBNET_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_object_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_rt_store.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/lib/slab/slab.hpp"

namespace pds_ms {

// Caches the Subnet spec as it is received from Protos in the North
// Lifecycle owned by Mgmt Stub instead of HAL stub
class subnet_obj_t : public slab_obj_t<subnet_obj_t>,
                     public base_obj_t {
public:
    struct properties_t {
        bool spec_invalid = false;
        bool hal_created = false;
    };
    subnet_obj_t(const pds_subnet_spec_t& spec) : pds_spec_(spec) {};
    pds_subnet_spec_t& spec(void) {return pds_spec_;}
    const pds_subnet_spec_t& spec(void) const {return pds_spec_;}
    properties_t& properties() {return prop_;}

public:
    // to store import RTs for ORF support
    rt_store_t rt_store;

private:  
    pds_subnet_spec_t  pds_spec_;  // PDS owned
                                   // Fields will not be modified by MS HAL Stubs 
    properties_t prop_;


};

class subnet_store_t : public obj_store_t <ms_bd_id_t, subnet_obj_t> {
};

void subnet_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);

} // End namespace

#endif
