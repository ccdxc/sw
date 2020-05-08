//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "ftltest_utils.hpp"

void
fill_key (uint32_t index, pds_l2_flow_key_t *key)
{
 
    uint8_t dmac[ETH_ADDR_LEN] = {0};
    memset(key, 0, sizeof(pds_l2_flow_key_t));
    key->vnic_id = index;
    dmac[0] = index & 0xff;
    dmac[2] = ((index + 63) % 512) & 0xff; 
    dmac[3] = (index >> 8) & 0xff;
    dmac[4] = 0xaa;
    memcpy(key->dmac, dmac,ETH_ADDR_LEN);
    return;
}

void
fill_data (uint32_t index,
           pds_l2_flow_data_t *data)
{
    memset(data, 0, sizeof(pds_l2_flow_data_t));
    data->index = index;
    return;
}

void
dump_flow (pds_l2_flow_iter_cb_arg_t *iter_cb_arg)
{
    pds_l2_flow_key_t *key = &iter_cb_arg->l2_flow_key;
    pds_l2_flow_data_t *data = &iter_cb_arg->l2_flow_appdata;

    printf("Dmac:%02x:%02x:%02x:%02x:%02x:%02x "
           "VNICID:%u "
           "index:%u \n\n",
           key->dmac[5], key->dmac[4],key->dmac[3],
           key->dmac[2], key->dmac[1],key->dmac[0],
           key->vnic_id, data->index);
    return;
}

void
dump_stats (pds_l2_flow_stats_t *stats)
{
    printf("\nPrinting L2_Flow cache statistics\n");
    printf("Insert %lu, Insert_fail_dupl %lu, Insert_fail %lu, "
           "Insert_fail_recirc %lu\n"
           "Remove %lu, Remove_not_found %lu, Remove_fail %lu\n"
           "Update %lu, Update_fail %lu\n"
           "Get %lu, Get_fail %lu\n"
           "Reserve %lu, reserve_fail %lu\n"
           "Release %lu, Release_fail %lu\n"
           "Tbl_entries %lu, Tbl_collision %lu\n"
           "Tbl_insert %lu, Tbl_remove %lu, Tbl_read %lu, Tbl_write %lu\n",
           stats->api_insert,
           stats->api_insert_duplicate,
           stats->api_insert_fail,
           stats->api_insert_recirc_fail,
           stats->api_remove,
           stats->api_remove_not_found,
           stats->api_remove_fail,
           stats->api_update,
           stats->api_update_fail,
           stats->api_get,
           stats->api_get_fail,
           stats->api_reserve,
           stats->api_reserve_fail,
           stats->api_release,
           stats->api_release_fail,
           stats->table_entries, stats->table_collisions,
           stats->table_insert, stats->table_remove,
           stats->table_read, stats->table_write);
    for (int i= 0; i < PDS_L2_FLOW_TABLE_MAX_RECIRC; i++) {
         printf("Tbl_lvl %u, Tbl_insert %lu, Tbl_remove %lu\n",
                 i, stats->table_insert_lvl[i], stats->table_remove_lvl[i]);
    }
    return;
}
