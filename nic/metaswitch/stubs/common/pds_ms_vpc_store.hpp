//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub VPC store
//---------------------------------------------------------------

#ifndef __PDS_MS_VPC_STORE_HPP__
#define __PDS_MS_VPC_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_object_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_rt_store.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/lib/slab/slab.hpp"

namespace pds_ms {

class vpc_obj_t : public slab_obj_t<vpc_obj_t>,
                  public base_obj_t {
public:
    struct properties_t {
        ms_vrf_id_t     vrf_id;
        pds_vpc_spec_t  vpc_spec;
        bool            hal_created = false; // Is the VPC created in HAL
        bool            spec_invalid = false; // Has the VPC Spec been deleted
                                              // mgmt

        properties_t(ms_vrf_id_t vrf_id_, const pds_vpc_spec_t& vpc_spec_)
            : vrf_id(vrf_id_), vpc_spec(vpc_spec_) {};
    };
    vpc_obj_t(ms_vrf_id_t vrf_id_, const pds_vpc_spec_t& vpc_spec_)
        : prop_(vrf_id_, vpc_spec_) {};
    void set_properties (const properties_t& prop) {prop_ = prop;}
    properties_t& properties(void) {return prop_;}
    const properties_t& properties(void) const {return prop_;}
    ms_vrf_id_t key(void) const {return prop_.vrf_id; }
    void update_store(state_t* state, bool op_delete) override;
    void print_debug_str(void) override {};

public:
    // to store import RTs for ORF support
    rt_store_t rt_store;

private:
    properties_t prop_;
};

class vpc_store_t : public obj_store_t <ms_vrf_id_t, vpc_obj_t> {
};

void vpc_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);

}

#endif
