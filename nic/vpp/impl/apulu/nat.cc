//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include <netinet/in.h>
#include <pthread.h>
#include <nic/sdk/asic/pd/pd.hpp>
#include <nic/sdk/asic/port.hpp>
#include <nic/sdk/include/sdk/table.hpp>
#include <nic/sdk/lib/p4/p4_utils.hpp>
#include <nic/sdk/lib/p4/p4_api.hpp>
#include <nic/sdk/platform/capri/capri_tbl_rw.hpp>
#include <nic/p4/common/defines.h>
#include <nic/apollo/p4/include/defines.h>
#include <nic/vpp/impl/nat.h>
#include <gen/p4gen/apulu/include/p4pd.h>

using namespace sdk;
using namespace sdk::table;
using namespace sdk::platform;

extern "C" {

p4pd_table_properties_t g_nat_tbl_ctx;

int
pds_nat_init(void)
{
    p4pd_table_properties_get(P4TBL_ID_NAT, &g_nat_tbl_ctx);
    return 0;
}

int
pds_snat_tbl_write_ip4(int nat_index, uint32_t ip, uint16_t port)
{
    // TODO : change to more efficient mmap and write
    nat_actiondata_t nat_data = { 0 };

    *(uint32_t *)&nat_data.action_u.nat_nat_rewrite.ip[0] = ip;
    nat_data.action_u.nat_nat_rewrite.port = port;

    p4pd_entry_write(P4TBL_ID_NAT, nat_index, NULL, NULL, &nat_data);

    return 0;
}

int
pds_dnat_tbl_write_ip4(int nat_index, uint32_t ip, uint16_t port)
{
    // TODO : change to more efficient mmap and write
    nat_actiondata_t nat_data = { 0 };

    *(uint32_t *)&nat_data.action_u.nat_nat_rewrite.ip[0] = ip;
    nat_data.action_u.nat_nat_rewrite.port = port;

    p4pd_entry_write(P4TBL_ID_NAT2, nat_index, NULL, NULL, &nat_data);

    return 0;
}

int
pds_snat_tbl_read_ip4(int nat_index, uint32_t *ip, uint16_t *port)
{
    // TODO : change to more efficient mmap and write
    nat_actiondata_t nat_data = { 0 };

    p4pd_entry_read(P4TBL_ID_NAT, nat_index, NULL, NULL, &nat_data);
    *ip = *(uint32_t *)&nat_data.action_u.nat_nat_rewrite.ip[0];
    *port = nat_data.action_u.nat_nat_rewrite.port;

    return 0;
}

int
pds_dnat_tbl_read_ip4(int nat_index, uint32_t *ip, uint16_t *port)
{
    // TODO : change to more efficient mmap and write
    nat_actiondata_t nat_data = { 0 };

    p4pd_entry_read(P4TBL_ID_NAT2, nat_index, NULL, NULL, &nat_data);
    *ip = *(uint32_t *)&nat_data.action_u.nat_nat_rewrite.ip[0];
    *port = nat_data.action_u.nat_nat_rewrite.port;

    return 0;
}

}
