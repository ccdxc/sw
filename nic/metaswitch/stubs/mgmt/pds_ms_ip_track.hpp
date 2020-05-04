// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __PDS_MS_IP_TRACK_HPP__
#define __PDS_MS_IP_TRACK_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/apollo/framework/api.h"
#include "nic/sdk/include/sdk/ip.hpp"

namespace pds_ms {

sdk_ret_t
ip_track_add (ip_addr_t& destip, obj_id_t pds_obj_id);

sdk_ret_t
ip_track_del (ip_addr_t& destip);

};

#endif    //__PDS_MS_VPC_HPP__

