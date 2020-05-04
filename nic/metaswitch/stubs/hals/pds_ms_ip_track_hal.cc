// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
// Purpose: Stitch tracked IP to underlay reachability in HAL

#include "nic/metaswitch/stubs/hals/pds_ms_ip_track_hal.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"

namespace pds_ms {

// Call-back from MS underlay route update
sdk_ret_t
ip_track_reachability_change (ip_addr_t& destip,
							  ms_hw_tbl_id_t nhgroup_id,
							  obj_id_t pds_obj_id) {

    PDS_TRACE_DEBUG("++++ IP Track %s reachability change to "
                    "Underlay NHgroup %d ++++",
                    ipaddr2str(&destip), nhgroup_id);

    // TODO Call HAL API based on obj type
    // pds_obj_key_t nh_group = msidx2pdsobjkey(nhgroup_id);

    switch (pds_obj_id) {
        case OBJ_ID_MIRROR_SESSION:
            break;
        case OBJ_ID_TEP:
            break;
        default:
            SDK_ASSERT(0);
    }
    return SDK_RET_OK;
}

} // End namespace
