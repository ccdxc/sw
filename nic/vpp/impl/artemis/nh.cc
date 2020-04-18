//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
//

#include <nic/apollo/api/impl/artemis/artemis_impl.hpp>
#include <nic/vpp/impl/nh.h>

uint16_t
pds_nh_drop_id_get (void)
{
    return PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID;
}

