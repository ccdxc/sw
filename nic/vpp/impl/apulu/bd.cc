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
pds_l2vnid_get(uint16_t bd_id, uint32_t *l2_vni) {
    p4pd_error_t p4pd_ret;
    bd_actiondata_t bd_data;

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_BD, bd_id, NULL, NULL,
                                      &bd_data);

    *l2_vni = bd_data.action_u.bd_bd_info.vni;

    return p4pd_ret;
}

#ifdef __cplusplus
}
#endif
