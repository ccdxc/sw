//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include <pthread.h>
#include <pi_impl.h>
#include <nic/apollo/p4/include/defines.h>
#include <nic/p4/common/defines.h>
#include <nic/sdk/asic/pd/pd.hpp>
#include <nic/sdk/include/sdk/table.hpp>
#include <nic/sdk/lib/p4/p4_utils.hpp>
#include <nic/sdk/lib/p4/p4_api.hpp>
#include <nic/sdk/platform/capri/capri_p4.hpp>
#include <nic/sdk/platform/capri/capri_tbl_rw.hpp>
#include <gen/p4gen/apollo/include/p4pd.h>

using namespace sdk;
using namespace sdk::table;
using namespace sdk::platform;

extern "C" {

pthread_key_t key;
p4pd_table_properties_t g_egress_vnic_tbl_ctx;
#define egress_local_vnic_info_action action_u.egress_local_vnic_info_egress_local_vnic_info
#define vnic_data_t egress_local_vnic_info_actiondata_t

// TODO - for optimised read
int
pds_init(void)
{
    (void) p4pd_table_properties_get(P4TBL_ID_EGRESS_LOCAL_VNIC_INFO,
                                     &g_egress_vnic_tbl_ctx);

    return 0;
}

int
egress_vnic_read(int vnic_id, vnic_t *vnic)
{
    p4pd_error_t p4pd_ret;
    vnic_data_t *egress_vnic_data = NULL;

    if (!(egress_vnic_data = (vnic_data_t*) malloc(sizeof(vnic_data_t)))) {
        SDK_ASSERT(0);
    }
    memset(egress_vnic_data, 0, sizeof(vnic_data_t));
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_EGRESS_LOCAL_VNIC_INFO, vnic_id,
                                      NULL, NULL, egress_vnic_data);
    memcpy(vnic->vr_mac,
           egress_vnic_data->egress_local_vnic_info_action.vr_mac, 6);
    vnic->vlan_id =
        egress_vnic_data->egress_local_vnic_info_action.overlay_vlan_id;

    return p4pd_ret;
}

}
