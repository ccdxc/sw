//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA stub VPC store
//---------------------------------------------------------------

#ifndef __PDSA_VPC_STORE_HPP__
#define __PDSA_VPC_STORE_HPP__

#include "nic/metaswitch/stubs/common/pdsa_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pdsa_object_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_slab_object.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/lib/slab/slab.hpp"

namespace pdsa_stub {

class vpc_obj_t : public slab_obj_t<vpc_obj_t>,
                  public base_obj_t {
public:
    struct properties_t {
        pds_vpc_spec_t  vpc_spec;
        properties_t(const pds_vpc_spec_t& vpc_spec_) : vpc_spec(vpc_spec_) {};
    };
    vpc_obj_t(const properties_t& prop) : prop_(prop) {};
    vpc_obj_t(const pds_vpc_spec_t& vpc_spec_) : prop_(vpc_spec_) {};
    void set_properties (const properties_t& prop) {prop_ = prop;}
    properties_t& properties(void) {return prop_;}
    const properties_t& properties(void) const {return prop_;}
    pds_vpc_id_t key(void) const {return prop_.vpc_spec.key.id;}
    void update_store(state_t* state, bool op_delete) override;
    void print_debug_str(void) override {};

private:  
    properties_t prop_;
};

class vpc_store_t : public obj_store_t <pds_vpc_id_t, vpc_obj_t> {
};

void vpc_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);

}

#endif
