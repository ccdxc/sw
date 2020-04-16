/*
* Copyright (c) 2020, Pensando Systems Inc.
*/

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "lib/table/common/table.hpp"
#include "lib/table/directmap/directmap.hpp"

#include "gen/platform/mem_regions.hpp"

#include "impl.hpp"

#ifdef IRIS
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd_table.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd_table.h"
#else
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd_table.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd_table.h"
#endif

#ifndef IRIS
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN            P4_P4PLUS_RXDMA_TBL_ID_INDEX_MIN
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX            P4_P4PLUS_RXDMA_TBL_ID_INDEX_MAX
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN            P4_P4PLUS_TXDMA_TBL_ID_INDEX_MIN
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX            P4_P4PLUS_TXDMA_TBL_ID_INDEX_MAX
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN               P4_P4PLUS_RXDMA_TBL_ID_TBLMIN
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX               P4_P4PLUS_RXDMA_TBL_ID_TBLMAX
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN               P4_P4PLUS_TXDMA_TBL_ID_TBLMIN
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX               P4_P4PLUS_TXDMA_TBL_ID_TBLMAX
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS    P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_PARAMS
#endif

#define ENTRY_TRACE_EN                      true


directmap    **p4plus_rxdma_dm_tables_;
directmap    **p4plus_txdma_dm_tables_;


void *
memrev(void *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = (uint8_t *)block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

void table_health_monitor(uint32_t table_id,
                          char *name,
                          table_health_state_t curr_state,
                          uint32_t capacity,
                          uint32_t usage,
                          table_health_state_t *new_state)
{
    printf("table id: %d, name: %s, capacity: %d, "
            "usage: %d, curr state: %d, new state: %d",
            table_id, name, capacity, usage, curr_state, *new_state);
}

int
p4plus_rxdma_init_tables()
{
    uint32_t                   tid;
    p4pd_table_properties_t    tinfo;
    p4pd_error_t               rc;
    std::string hal_cfg_path_ = hal_cfg_path();

#if defined(APOLLO)
    p4pd_cfg_t                 p4pd_cfg = {
            .table_map_cfg_file  = "apollo/capri_rxdma_table_map.json",
            .p4pd_pgm_name       = "apollo",
            .p4pd_rxdma_pgm_name = "p4plus",
            .p4pd_txdma_pgm_name = "p4plus",
            .cfg_path            = hal_cfg_path_.c_str(),
    };
#elif defined(ARTEMIS)
    p4pd_cfg_t                 p4pd_cfg = {
            .table_map_cfg_file  = "artemis/capri_rxdma_table_map.json",
            .p4pd_pgm_name       = "artemis",
            .p4pd_rxdma_pgm_name = "p4plus",
            .p4pd_txdma_pgm_name = "p4plus",
            .cfg_path            = hal_cfg_path_.c_str(),
    };
#elif defined(APULU)
    p4pd_cfg_t                 p4pd_cfg = {
            .table_map_cfg_file  = "apulu/capri_rxdma_table_map.json",
            .p4pd_pgm_name       = "apulu",
            .p4pd_rxdma_pgm_name = "p4plus",
            .p4pd_txdma_pgm_name = "p4plus",
            .cfg_path            = hal_cfg_path_.c_str(),
    };
#elif defined(ATHENA)
    p4pd_cfg_t                 p4pd_cfg = {
            .table_map_cfg_file  = "athena/capri_rxdma_table_map.json",
            .p4pd_pgm_name       = "apollo", /* FIXME - change to athena when ready */
            .p4pd_rxdma_pgm_name = "p4plus",
            .p4pd_txdma_pgm_name = "p4plus",
            .cfg_path            = hal_cfg_path_.c_str(),
    };
#else
    p4pd_cfg_t                 p4pd_cfg = {
            .table_map_cfg_file  = "iris/capri_p4_rxdma_table_map.json",
            .p4pd_pgm_name       = "iris",
            .p4pd_rxdma_pgm_name = "p4plus",
            .p4pd_txdma_pgm_name = "p4plus",
            .cfg_path            = hal_cfg_path_.c_str(),
    };
#endif

    memset(&tinfo, 0, sizeof(tinfo));

    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_rxdma_init(&p4pd_cfg);
    assert(rc == P4PD_SUCCESS);

    // start instantiating tables based on the parsed information
    p4plus_rxdma_dm_tables_ =
            (directmap **)calloc(sizeof(directmap *),
            (P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX -
             P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    assert(p4plus_rxdma_dm_tables_ != NULL);

    // TODO:
    // 1. take care of instantiating flow_table_, acl_table_ and met_table_
    // 2. When tables are instantiated proper names are not passed today,
    // waiting for an API from Mahesh that gives table name given table id

    for (tid = P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_rxdma_table_properties_get(tid, &tinfo);
        assert(rc == P4PD_SUCCESS);

        switch (tinfo.table_type) {
            case P4_TBL_TYPE_INDEX:
                if (tinfo.tabledepth) {
                    p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                        directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                           false, ENTRY_TRACE_EN, table_health_monitor);
                    assert(p4plus_rxdma_dm_tables_
                           [tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
                }
                break;

            case P4_TBL_TYPE_MPU:
            default:
                break;
        }
    }

    return 0;
}

int
p4plus_txdma_init_tables()
{
    uint32_t                   tid;
    p4pd_table_properties_t    tinfo;
    p4pd_error_t               rc;
    std::string hal_cfg_path_ = hal_cfg_path();

#if defined(APOLLO)
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "apollo/capri_txdma_table_map.json",
        .p4pd_pgm_name       = "apollo",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = hal_cfg_path_.c_str(),
    };
#elif defined(ARTEMIS)
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "artemis/capri_txdma_table_map.json",
        .p4pd_pgm_name       = "artemis",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = hal_cfg_path_.c_str(),
    };
#elif defined(APULU)
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "apulu/capri_txdma_table_map.json",
        .p4pd_pgm_name       = "apulu",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = hal_cfg_path_.c_str(),
    };
#elif defined(ATHENA)
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "athena/capri_txdma_table_map.json",
        .p4pd_pgm_name       = "apollo", /* FIXME - change to athena when ready */
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = hal_cfg_path_.c_str(),
    };
#else
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "iris/capri_p4_txdma_table_map.json",
        .p4pd_pgm_name       = "iris",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = hal_cfg_path_.c_str(),
    };
#endif

    memset(&tinfo, 0, sizeof(tinfo));

    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_txdma_init(&p4pd_cfg);
    assert(rc == P4PD_SUCCESS);

    // start instantiating tables based on the parsed information
    p4plus_txdma_dm_tables_ =
        (directmap **)calloc(sizeof(directmap *),
                             (P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX -
                              P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    SDK_ASSERT(p4plus_txdma_dm_tables_ != NULL);

    // TODO:
    // 1. take care of instantiating flow_table_, acl_table_ and met_table_
    // 2. When tables are instantiated proper names are not passed today,
    // waiting for an API from Mahesh that gives table name given table id

    for (tid = P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_txdma_table_properties_get(tid, &tinfo);
        SDK_ASSERT(rc == P4PD_SUCCESS);

        switch (tinfo.table_type) {
        case P4_TBL_TYPE_INDEX:
            if (tinfo.tabledepth) {
                p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                    directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                       false, ENTRY_TRACE_EN, table_health_monitor);
                assert(p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
            }
            break;

        case P4_TBL_TYPE_MPU:
        default:
            break;
        }
    }

    return 0;
}

void
pd_init()
{
    int ret;
    sdk::asic::pd::asicpd_state_pd_init(NULL);

    ret = p4plus_rxdma_init_tables();
    assert(ret == 0);
    ret = p4plus_txdma_init_tables();
    assert(ret == 0);

#ifdef IRIS
    ret = sdk::asic::pd::asicpd_p4plus_table_rw_init();
    assert(ret == 0);
#endif
}

// Maximum number of queue per LIF
#define ETH_RSS_MAX_QUEUES                  (128)
// Number of entries in a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_LEN           ETH_RSS_MAX_QUEUES
// Size of each LIF indirection table entry
#define ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ      (sizeof(eth_rx_rss_indir_eth_rx_rss_indir_t))
// Size of a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_SZ            (ETH_RSS_LIF_INDIR_TBL_LEN * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ)
// Max number of LIFs supported
#define MAX_LIFS                            (2048)
// Size of the entire LIF indirection table
#define ETH_RSS_INDIR_TBL_SZ                (MAX_LIFS * ETH_RSS_LIF_INDIR_TBL_SZ)
// Memory bar should be multiple of 8 MB
#define MEM_BARMAP_SIZE_SHIFT               (23)

static int
p4pd_common_p4plus_rxdma_rss_params_table_entry_get(uint32_t hw_lif_id,
    void *data)
{
    p4pd_error_t        pd_err;

    assert(hw_lif_id < MAX_LIFS);

    pd_init();

    pd_err = p4pd_global_entry_read(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS,
                                    hw_lif_id, NULL, NULL, data);
    if (pd_err != P4PD_SUCCESS) {
        assert(0);
    }

    return 0;
}

void
p4pd_common_p4plus_rxdma_rss_params_table_entry_show(uint32_t hw_lif_id)
{
    eth_rx_rss_params_actiondata_t data = { 0 };

    p4pd_common_p4plus_rxdma_rss_params_table_entry_get(hw_lif_id, &data);

    memrev((uint8_t *)&data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           sizeof(data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));

    printf("type:  %x\n", data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_type);
    printf("key:   ");
    for (uint16_t i = 0; i < sizeof(data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key); i++) {
        if (i != 0)
            printf(":");
        printf("%02x", data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key[i]);
    }
    printf("\n");
    printf("debug: %x\n", data.action_u.eth_rx_rss_params_eth_rx_rss_params.debug);
}

int
p4pd_common_p4plus_rxdma_rss_params_table_entry_add(uint32_t hw_lif_id,
    uint8_t debug)
{
    p4pd_error_t        pd_err;
    eth_rx_rss_params_actiondata_t data = { 0 };

    assert(hw_lif_id < MAX_LIFS);

    p4pd_common_p4plus_rxdma_rss_params_table_entry_get(hw_lif_id, &data);

    data.action_u.eth_rx_rss_params_eth_rx_rss_params.debug = debug;

    pd_err = p4pd_global_entry_write(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS,
                                     hw_lif_id, NULL, NULL, &data);
    if (pd_err != P4PD_SUCCESS) {
        assert(0);
    }

    return 0;
}

int
p4pd_common_p4plus_rxdma_rss_indir_table_entry_show(uint32_t hw_lif_id)
{
    uint64_t tbl_base;
    uint64_t tbl_index;
    uint64_t addr;
    uint8_t index;
    eth_rx_rss_indir_actiondata_t data;

    std::string mpart_json = mpart_cfg_path();
    mpartition *mp_ = mpartition::factory(mpart_json.c_str());
    assert(mp_);

    if (hw_lif_id >= MAX_LIFS) {
        printf("Invalid lif %d\n", hw_lif_id);
        return -1;
    };

    tbl_base = mp_->start_addr(MEM_REGION_RSS_INDIR_TABLE_NAME);
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);
    tbl_base += (hw_lif_id * ETH_RSS_LIF_INDIR_TBL_SZ);

    printf("ind_table: 0x%lx\n\t", tbl_base);
    for (index = 0; index < ETH_RSS_MAX_QUEUES; index++) {
        tbl_index = (index * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ);
        addr = tbl_base + tbl_index;
        sdk::lib::pal_mem_read(addr, (uint8_t *)&data.action_u,
                        sizeof(data.action_u), 0);
        printf("[%3d] %3d ", index,
            data.action_u.eth_rx_rss_indir_eth_rx_rss_indir.qid);
        if ((index + 1) % 8 == 0)
            printf("\n\t");
    }

    return 0;
}
