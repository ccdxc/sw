//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub store for destination ip addresses whose underlay reachability
// is tracked
//---------------------------------------------------------------

#ifndef __PDS_MS_IP_TRACK_STORE_HPP__
#define __PDS_MS_IP_TRACK_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ecmp_idx_guard.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_object_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/lib/slab/slab.hpp"

namespace pds_ms {

// Destination IP whose underlay reachability is tracked
class ip_track_obj_t : public slab_obj_t<ip_track_obj_t>,
                           public base_obj_t {
public:
    ip_track_obj_t(const ip_addr_t& destip,
                       obj_id_t pds_obj_id);
    ~ip_track_obj_t(void);

    // Internal IP used to create static routes
    // for tracking destination IP
    ip_prefix_t internal_ip_prefix(void);

    ip_addr_t& destip(void) {return destip_;}
    obj_id_t pds_obj_id(void) {return pds_obj_id_;}

private:  
    ip_addr_t  destip_; // IP whose reachability is being tracked
    obj_id_t   pds_obj_id_;
    // Index generator for internal IP used to create static routes
    // for tracking destination IP
    uint32_t internal_index_;

};

// Tracked DestIP -> DestIP Track Obj
class ip_track_store_t : public obj_store_t <ip_addr_t,
                                             ip_track_obj_t,
                                             ip_hash> {
};

void ip_track_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);

// InternalIP -> Tracked DestIP 
using ip_track_internalip_store_t =
               std::unordered_map<ip_prefix_t, ip_addr_t, ip_prefix_hash>;

} // End namespace

#endif
