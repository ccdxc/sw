// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __PDS_MS_IP_TRACK_HAL_HPP__
#define __PDS_MS_IP_TRACK_HAL_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/apollo/framework/api.h"
#include "nic/sdk/include/sdk/ip.hpp"

namespace pds_ms {

// Call-back from MS underlay route update
sdk_ret_t
ip_track_reachability_change (ip_addr_t& destip,
							  ms_hw_tbl_id_t nhgroup_id,
							  obj_id_t pds_obj_id);

};

#endif

