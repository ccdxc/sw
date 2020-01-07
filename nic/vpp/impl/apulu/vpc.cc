//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
//

#include <nic/sdk/lib/utils/utils.hpp>
#include <gen/p4gen/apulu/include/p4pd.h>
#include <nic/apollo/p4/include/apulu_defines.h>
#include <nic/sdk/lib/p4/p4_utils.hpp>
#include <nic/sdk/lib/p4/p4_api.hpp>
#ifdef __cplusplus
extern "C" {
#endif

int
pds_l3vnid_get(uint16_t vpc_id, uint32_t *l3_vni) {
    p4pd_error_t p4pd_ret;
    vpc_actiondata_t vpc_data;

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VPC, vpc_id, NULL, NULL,
                                      &vpc_data);

    *l3_vni = vpc_data.action_u.vpc_vpc_info.vni;

    return p4pd_ret;
}
#ifdef __cplusplus 
}
#endif
