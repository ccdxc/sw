//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA stub Overlay Bridge Domain store
//---------------------------------------------------------------

#ifndef __PDSA_BD_STORE_HPP__
#define __PDSA_BD_STORE_HPP__

#include "nic/metaswitch/stubs/common/pdsa_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pdsa_object_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_slab_object.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/lib/slab/slab.hpp"

namespace pdsa_stub {

class bd_obj_t : public slab_obj_t<bd_obj_t> {
public:
    struct properties_t {
        ms_bd_id_t        bd_id;
        pds_vnid_id_t     vni;
        pds_subnet_id_t   hal_idx;
        properties_t(ms_bd_id_t bd_id_, pds_vnid_id_t vni_, pds_subnet_id_t hal_idx_) 
            : bd_id(bd_id_), vni(vni_), hal_idx(hal_idx_) {};
    };

    bd_obj_t(const properties_t& prop) : prop_(prop) {};
    void set_properties(const properties_t& prop) {prop_ = prop;}
    const properties_t& properties(void) const {return prop_;}

private:  
    properties_t prop_;
};

class bd_store_t : public obj_store_t <ms_bd_id_t, bd_obj_t> {
};

}

#endif
