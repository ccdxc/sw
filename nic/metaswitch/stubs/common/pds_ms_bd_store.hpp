//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub Overlay Bridge Domain store
//---------------------------------------------------------------

#ifndef __PDS_MS_BD_STORE_HPP__
#define __PDS_MS_BD_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_object_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/lib/slab/slab.hpp"

namespace pds_ms {

class bd_obj_t : public slab_obj_t<bd_obj_t>,
                 public base_obj_t {
public:
    struct properties_t {
        pds_subnet_spec_t  pds_spec;        // PDS owned
        pds_encap_t        fabric_encap;    // MS owned
        pds_ifindex_t      host_ifindex;    // MS owned
        bool               hal_created = false;
        properties_t(const pds_subnet_spec_t& pds_spec_) 
            : pds_spec(pds_spec_) {};
    };

    bd_obj_t(const pds_subnet_spec_t& spec) : prop_(spec) {};
    void set_properties(const properties_t& prop) {prop_ = prop;}
    properties_t& properties(void) {return prop_;}
    const properties_t& properties(void) const {return prop_;}
    ms_bd_id_t key(void) const {return prop_.pds_spec.key.id;}
    void update_store(state_t* state, bool op_delete) override;
    void print_debug_str(void) override {};

private:  
    properties_t prop_;
};

class bd_store_t : public obj_store_t <ms_bd_id_t, bd_obj_t> {
};

void bd_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);

}

#endif
