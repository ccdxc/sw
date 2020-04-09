//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub indirect pathset store
//---------------------------------------------------------------

#ifndef __PDS_MS_INDIRECT_PS_STORE_HPP__
#define __PDS_MS_INDIRECT_PS_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_object_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/sdk/lib/slab/slab.hpp"

namespace pds_ms {

// Indirect Pathset (Cascaded mode) that points to direct pathsets
// Used for underlay nexthops that are used by VXLAN Tunnels
// Holds back-ref from indirect pathset to TEP
class indirect_ps_obj_t : public slab_obj_t<indirect_ps_obj_t>,
                 public base_obj_t {
public:
    indirect_ps_obj_t();
    indirect_ps_obj_t(ms_ps_id_t ll_direct_ps);
    indirect_ps_obj_t(const ip_addr_t& tep);

public:
    ms_ps_id_t   ll_direct_pathset = 0; // direct pathset ID
    ip_addr_t  tep_ip;       // TEP IP
};

class indirect_ps_store_t : public obj_store_t <ms_ps_id_t, indirect_ps_obj_t> {
};

void indirect_ps_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);

} // End namespace

#endif

