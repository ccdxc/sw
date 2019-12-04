// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __PDSA_LIM_UTILS_HPP__
#define __PDSA_LIM_UTILS_HPP__

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_vpc.hpp"

namespace pdsa_stub {

NBB_VOID pdsa_row_update_lim_vrf (pds_vpc_id_t   vpc_id, 
                                  NBB_LONG       row_status, 
                                  NBB_ULONG      correlator);

NBB_VOID pdsa_row_update_lim_gen_irb_if (pds_subnet_id_t bd_index,
                                         NBB_LONG        row_status,
                                         NBB_ULONG       correlator);
}
#endif /*__PDSA_LIM_UTILS_HPP__*/
