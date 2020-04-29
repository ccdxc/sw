// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __PDS_MS_DESTIP_TRACK_HPP__
#define __PDS_MS_DESTIP_TRACK_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/apollo/framework/api.h"
#include "nic/sdk/include/sdk/ip.hpp"

namespace pds_ms {


sdk_ret_t
destip_track_start (ip_addr_t& destip_track, obj_id_t pds_obj_id);

sdk_ret_t
destip_track_stop (ip_addr_t& destip_track);

// Call-back from MS underlay route update
sdk_ret_t
destip_track_reachability_change (ip_addr_t& destip_track,
                                  ms_hw_tbl_id_t nhgroup_id,
                                  obj_id_t pds_obj_id);

};

#endif    //__PDS_MS_VPC_HPP__

