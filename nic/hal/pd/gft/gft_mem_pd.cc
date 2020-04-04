// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.hpp"
#include "nic/hal/pd/gft/gft_state.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/pd/gft/lif_pd.hpp"
#include "nic/hal/pd/gft/uplinkif_pd.hpp"
#include "nic/hal/pd/gft/enicif_pd.hpp"
#include "nic/hal/pd/gft/endpoint_pd.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"
#include "nic/hal/pd/gft/emp_pd.hpp"
#include "nic/hal/pd/gft/vrf_pd.hpp"
#include "nic/hal/pd/gft/efe_pd.hpp"
#include "nic/hal/pd/hal_pd.hpp"

namespace hal {
namespace pd {

// P4PD_Lib_Trace Vs Table_Lib_Trace
// ---------------------------------
//  Table_Lib_Trace = !P4PD_Lib_Trace
//  Effectively don't want dual tracing or no tracing.
// #define ENTRY_TRACE_EN SDK_LOG_TABLE_WRITE ? false : true
#define ENTRY_TRACE_EN true

class hal_state_pd *g_hal_state_pd;

bool
hal_state_pd::init(void)
{
    dm_tables_              = NULL;
    hash_tcam_tables_       = NULL;
    tcam_tables_            = NULL;
    flow_table_             = NULL;
    tx_flow_table_          = NULL;
    p4plus_rxdma_dm_tables_ = NULL;
    p4plus_txdma_dm_tables_ = NULL;

    // initialize LIF PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_LIF_PD)] =
        slab::factory("LIF_PD", HAL_SLAB_LIF_PD,
                      sizeof(hal::pd::pd_lif_t), 8,
                      false, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_LIF_PD)] != NULL),
                      false);

    lif_hwid_idxr_ = sdk::lib::indexer::factory(HAL_MAX_HW_LIFS);
    SDK_ASSERT_RETURN((lif_hwid_idxr_ != NULL), false);

    // initialize vrf related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_VRF_PD)] =
        slab::factory("Vrf PD", HAL_SLAB_VRF_PD,
                      sizeof(hal::pd::pd_vrf_t), 8,
                      false, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_VRF_PD)] != NULL),
                      false);

    // initialize Uplink If PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_UPLINKIF_PD)] =
        slab::factory("UPLINKIF_PD", HAL_SLAB_UPLINKIF_PD,
                      sizeof(hal::pd::pd_uplinkif_t), 8,
                      false, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_UPLINKIF_PD)] != NULL),
                      false);

    // initialize ENIC If PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_ENICIF_PD)] =
        slab::factory("ENICIF_PD", HAL_SLAB_ENICIF_PD,
                      sizeof(hal::pd::pd_enicif_t), 8,
                      false, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_ENICIF_PD)] != NULL),
                      false);

    // initialize EP PD related data structures
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_EP_PD)] =
        slab::factory("EP_PD", HAL_SLAB_EP_PD,
                      sizeof(hal::pd::pd_ep_t), 8,
                      false, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_EP_PD)] != NULL),
                      false);

    // initiate GFT exact match profile
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_GFT_EMP_PD)] =
        slab::factory("GFT_EMP_PD",
                      HAL_SLAB_GFT_EMP_PD,
                      sizeof(hal::pd::pd_gft_emp_t),
                      8, false, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_GFT_EMP_PD)] != NULL),
                      false);

    // initiate GFT exact flow entry
    slabs_[HAL_PD_SLAB_ID(HAL_SLAB_GFT_EFE_PD)] =
        slab::factory("GFT_EFE_PD",
                      HAL_SLAB_GFT_EFE_PD,
                      sizeof(hal::pd::pd_gft_efe_t),
                      8, false, true, true);
    SDK_ASSERT_RETURN((slabs_[HAL_PD_SLAB_ID(HAL_SLAB_GFT_EFE_PD)] != NULL),
                      false);

    return true;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_state_pd::hal_state_pd()
{
    memset(slabs_, 0, sizeof(slabs_));
    lif_hwid_idxr_ = NULL;
}

//------------------------------------------------------------------------------
// (private) destructor method
//------------------------------------------------------------------------------
hal_state_pd::~hal_state_pd()
{
    uint32_t    tid;
    for (uint32_t i = HAL_SLAB_PD_MIN; i < HAL_SLAB_PD_MAX; i++) {
        if (slabs_[HAL_PD_SLAB_ID(i)]) {
            slab::destroy(slabs_[HAL_PD_SLAB_ID(i)]);
        }
    }

    lif_hwid_idxr_ ? indexer::destroy(lif_hwid_idxr_) : HAL_NOP;

    if (dm_tables_) {
        for (tid = P4TBL_ID_INDEX_MIN; tid < P4TBL_ID_INDEX_MAX; tid++) {
            if (dm_tables_[tid]) {
                directmap::destroy(dm_tables_[tid]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, dm_tables_);
    }

    if (hash_tcam_tables_) {
        for (tid = P4TBL_ID_HASH_OTCAM_MIN;
             tid < P4TBL_ID_HASH_OTCAM_MAX; tid++) {
            if (hash_tcam_tables_[tid]) {
                sdk_hash::destroy(hash_tcam_tables_[tid]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, hash_tcam_tables_);
    }

    if (tcam_tables_) {
        for (tid = P4TBL_ID_TCAM_MIN; tid < P4TBL_ID_TCAM_MIN; tid++) {
            if (tcam_tables_[tid]) {
                tcam::destroy(tcam_tables_[tid]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, tcam_tables_);
    }

    if (flow_table_) {
        HbmHash::destroy(flow_table_);
    }

    if (tx_flow_table_) {
        HbmHash::destroy(tx_flow_table_);
    }

    if (p4plus_rxdma_dm_tables_) {
        for (tid = P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN;
             tid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX; tid++) {
            if (p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN]) {
                directmap::destroy(p4plus_rxdma_dm_tables_[tid -
                                   P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, p4plus_rxdma_dm_tables_);
    }

    if (p4plus_txdma_dm_tables_) {
        for (tid = P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN;
             tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX; tid++) {
            if (p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN]) {
                directmap::destroy(p4plus_txdma_dm_tables_[tid -
                                   P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN]);
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, p4plus_txdma_dm_tables_);
    }
}

void
hal_state_pd::destroy(hal_state_pd *state)
{
    if (!state) {
        return;
    }
    state->~hal_state_pd();
    HAL_FREE(HAL_MEM_ALLOC_PD, state);
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_state_pd *
hal_state_pd::factory(void)
{
    void            *mem;
    hal_state_pd    *new_state;

    mem = HAL_CALLOC(HAL_MEM_ALLOC_PD, sizeof(hal_state_pd));
    if (mem == NULL) {
        HAL_TRACE_ERR("Failed to instantiate GFT PD state\n");
        return NULL;
    }

    new_state = new (mem) hal_state_pd();
    if (new_state->init() == false) {
        new_state->~hal_state_pd();
        HAL_FREE(HAL_MEM_ALLOC_PD, mem);
        return NULL;
    }

    return new_state;
}

hal_ret_t
pd_get_slab (pd_func_args_t *pd_func_args)
{
    pd_get_slab_args_t *args = pd_func_args->pd_get_slab;
    args->slab = g_hal_state_pd->get_slab(args->slab_id);
    return HAL_RET_OK;
}

slab *
hal_state_pd::get_slab(hal_slab_t slab_id)
{
    if (slab_id >= HAL_SLAB_PD_MAX || slab_id < HAL_SLAB_PD_MIN) {
        return NULL;
    }
    return slabs_[HAL_PD_SLAB_ID(slab_id)];
    return NULL;
}

//------------------------------------------------------------------------------
// initializing tables
//------------------------------------------------------------------------------
hal_ret_t
hal_state_pd::init_tables(pd_mem_init_args_t *args)
{
    uint32_t                   tid;
    hal_ret_t                  ret = HAL_RET_OK;
    p4pd_table_properties_t    tinfo, ctinfo;
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "gft/capri_p4_table_map.json",
        .p4pd_pgm_name       = "gft",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = args->cfg_path
    };

    memset(&tinfo, 0, sizeof(tinfo));
    memset(&ctinfo, 0, sizeof(ctinfo));

    // parse the NCC generated table info file
    p4pd_init(&p4pd_cfg);

    // start instantiating tables based on the parsed information
    dm_tables_ =
        (directmap **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                                 sizeof(directmap *) *
                                 (P4TBL_ID_INDEX_MAX - P4TBL_ID_INDEX_MIN + 1));
    SDK_ASSERT(dm_tables_ != NULL);

    hash_tcam_tables_ =
        (sdk_hash **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                         sizeof(sdk_hash *) *
                         (P4TBL_ID_HASH_OTCAM_MAX - P4TBL_ID_HASH_OTCAM_MIN + 1));
    SDK_ASSERT(hash_tcam_tables_ != NULL);

    tcam_tables_ =
        (tcam **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                            sizeof(tcam *) *
                            (P4TBL_ID_TCAM_MAX - P4TBL_ID_TCAM_MIN + 1));
    SDK_ASSERT(tcam_tables_ != NULL);

    // for debugging
    //p4pd_table_info_dump_();

    for (tid = P4TBL_ID_TBLMIN; tid < P4TBL_ID_TBLMAX; tid++) {
        p4pd_table_properties_get(tid, &tinfo);
        switch (tinfo.table_type) {
        case P4_TBL_TYPE_HASHTCAM:
            if (tinfo.has_oflow_table) {
                p4pd_table_properties_get(tinfo.oflow_table_id, &ctinfo);
            }
            hash_tcam_tables_[tid - P4TBL_ID_HASH_OTCAM_MIN] =
                sdk_hash::factory(tinfo.tablename, tid,
                              tinfo.oflow_table_id,
                              tinfo.tabledepth,
                              tinfo.has_oflow_table ? ctinfo.tabledepth : 0,
                              tinfo.key_struct_size,
                              tinfo.actiondata_struct_size,
                              static_cast<sdk_hash::HashPoly>(tinfo.hash_type),
                              ENTRY_TRACE_EN);
            SDK_ASSERT(hash_tcam_tables_[tid - P4TBL_ID_HASH_OTCAM_MIN] != NULL);
            break;

        case P4_TBL_TYPE_TCAM:
            if (!tinfo.is_oflow_table) {
                tcam_tables_[tid - P4TBL_ID_TCAM_MIN] =
                    tcam::factory(tinfo.tablename, tid, tinfo.tabledepth,
                                  tinfo.key_struct_size,
                                  tinfo.actiondata_struct_size, true,
                                  ENTRY_TRACE_EN);
                SDK_ASSERT(tcam_tables_[tid - P4TBL_ID_TCAM_MIN] != NULL);
            }
            break;

        case P4_TBL_TYPE_INDEX:
            dm_tables_[tid - P4TBL_ID_INDEX_MIN] =
                directmap::factory(tinfo.tablename, tid, tinfo.tabledepth,
                                   tinfo.actiondata_struct_size, false,
                                   ENTRY_TRACE_EN);
            SDK_ASSERT(dm_tables_[tid - P4TBL_ID_INDEX_MIN] != NULL);
            break;

        case P4_TBL_TYPE_HASH:
            // TODO: May need another flow_table for TX_GFT_HASH
            if (tid == P4TBL_ID_RX_GFT_HASH) {
                if (tinfo.has_oflow_table) {
                    p4pd_table_properties_get(tinfo.oflow_table_id, &ctinfo);
                }
                flow_table_ =
                    HbmHash::factory(tinfo.tablename, tid, tinfo.oflow_table_id,
                                  tinfo.tabledepth, ctinfo.tabledepth,
                                  tinfo.key_struct_size,
                                  sizeof(gft_flow_hash_data_t), 10,    // no. of hints
                                  8, // Max # of recircs
                                  static_cast<HbmHash::HashPoly>(tinfo.hash_type),
                                  HAL_MEM_ALLOC_FLOW, ENTRY_TRACE_EN);
                SDK_ASSERT(flow_table_ != NULL);
            }
            if (tid == P4TBL_ID_TX_GFT_HASH) {
                if (tinfo.has_oflow_table) {
                    p4pd_table_properties_get(tinfo.oflow_table_id, &ctinfo);
                }
                tx_flow_table_ =
                    HbmHash::factory(tinfo.tablename, tid, tinfo.oflow_table_id,
                                  tinfo.tabledepth, ctinfo.tabledepth,
                                  tinfo.key_struct_size,
                                  sizeof(gft_flow_hash_data_t), 10,    // no. of hints
                                  8, // Max # of recircs
                                  static_cast<HbmHash::HashPoly>(tinfo.hash_type),
                                  HAL_MEM_ALLOC_FLOW, ENTRY_TRACE_EN);
                SDK_ASSERT(tx_flow_table_ != NULL);
            }
            break;

        case P4_TBL_TYPE_MPU:
        default:
            break;
        }
    }

    return ret;
}

hal_ret_t
hal_state_pd::p4plus_rxdma_init_tables(pd_mem_init_args_t *args)
{
    uint32_t                   tid;
    hal_ret_t                  ret = HAL_RET_OK;
    p4pd_table_properties_t    tinfo, ctinfo;
    p4pd_error_t               rc;
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "gft/capri_p4_rxdma_table_map.json",
        .p4pd_pgm_name       = "gft",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = args->cfg_path
    };

    memset(&tinfo, 0, sizeof(tinfo));
    memset(&ctinfo, 0, sizeof(ctinfo));

    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_rxdma_init(&p4pd_cfg);
    SDK_ASSERT(rc == P4PD_SUCCESS);

    // start instantiating tables based on the parsed information
    p4plus_rxdma_dm_tables_ =
        (directmap **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                                 sizeof(directmap *) *
                                 (P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX -
                                  P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    SDK_ASSERT(p4plus_rxdma_dm_tables_ != NULL);

    for (tid = P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_rxdma_table_properties_get(tid, &tinfo);
        SDK_ASSERT(rc == P4PD_SUCCESS);

        switch (tinfo.table_type) {
        case P4_TBL_TYPE_INDEX:
            p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                   false, ENTRY_TRACE_EN);
            SDK_ASSERT(p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
            break;

        case P4_TBL_TYPE_MPU:
        default:
            break;
        }
    }

    return ret;
}

hal_ret_t
hal_state_pd::p4plus_txdma_init_tables(pd_mem_init_args_t *args)
{
    uint32_t                   tid;
    hal_ret_t                  ret = HAL_RET_OK;
    p4pd_table_properties_t    tinfo, ctinfo;
    p4pd_error_t               rc;
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "gft/capri_p4_txdma_table_map.json",
        .p4pd_pgm_name       = "gft",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = args->cfg_path
    };

    memset(&tinfo, 0, sizeof(tinfo));
    memset(&ctinfo, 0, sizeof(ctinfo));

    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_txdma_init(&p4pd_cfg);
    SDK_ASSERT(rc == P4PD_SUCCESS);

    // start instantiating tables based on the parsed information
    p4plus_txdma_dm_tables_ =
        (directmap **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                                 sizeof(directmap *) *
                                 (P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX -
                                  P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    SDK_ASSERT(p4plus_txdma_dm_tables_ != NULL);

    for (tid = P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_txdma_table_properties_get(tid, &tinfo);
        SDK_ASSERT(rc == P4PD_SUCCESS);

        switch (tinfo.table_type) {
        case P4_TBL_TYPE_INDEX:
            p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                   false, ENTRY_TRACE_EN);
            SDK_ASSERT(p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
            break;

        case P4_TBL_TYPE_MPU:
        default:
            break;
        }
    }

    return ret;
}

hal_ret_t
pd_mem_init (pd_func_args_t *pd_func_args)
{
    hal_ret_t    ret = HAL_RET_OK;
    pd_mem_init_args_t *args = pd_func_args->pd_mem_init;

    g_hal_state_pd = hal_state_pd::factory();
    if (g_hal_state_pd == NULL) {
        HAL_TRACE_ERR("Failed to create HAL GFT PD state");
        return HAL_RET_OOM;
    }

    HAL_TRACE_DEBUG("Initializing p4 asic lib tables ...");
    ret = g_hal_state_pd->init_tables(args);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }

    HAL_TRACE_DEBUG("Initializing p4plus asic lib tables ...");
    ret = g_hal_state_pd->p4plus_rxdma_init_tables(args);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }

    HAL_TRACE_DEBUG("Initializing p4plus asic lib tables ...");
    ret = g_hal_state_pd->p4plus_txdma_init_tables(args);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }

    return HAL_RET_OK;

cleanup:

    hal_state_pd::destroy(g_hal_state_pd);
    return ret;
}

hal_ret_t
pd_mem_init_phase2 (pd_func_args_t *pd_func_args)
{
    // pd_mem_init_phase2_args_t *arg = pd_func_args->pd_mem_init_phase2;
    p4pd_cfg_t    p4pd_cfg = {
        .table_map_cfg_file  = "gft/capri_p4_table_map.json",
        .p4pd_pgm_name       = "gft",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
    };

    pd_mem_init_phase2_args_t           *ph2_args;
    hal::hal_cfg_t                      *hal_cfg;

    ph2_args = pd_func_args->pd_mem_init_phase2;
    hal_cfg = ph2_args->hal_cfg;

    // gft specific capri inits
    SDK_ASSERT(sdk::asic::pd::asicpd_p4plus_table_mpu_base_init(&p4pd_cfg) == SDK_RET_OK);
    SDK_ASSERT(sdk::asic::pd::asicpd_toeplitz_init("gft_rxdma",
                                    P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_INDIR,
                                    sizeof(eth_rx_rss_indir_eth_rx_rss_indir_t)) ==
               SDK_RET_OK);
    SDK_ASSERT(asicpd_p4plus_table_init(hal_cfg) == HAL_RET_OK);
    SDK_ASSERT(sdk::asic::pd::asicpd_p4plus_recirc_init() == SDK_RET_OK);

    // common asic pd init (must be called after capri asic init)
    SDK_ASSERT(sdk::asic::pd::asicpd_table_mpu_base_init(&p4pd_cfg) == SDK_RET_OK);
    SDK_ASSERT(sdk::asic::pd::asicpd_program_table_mpu_pc() == SDK_RET_OK);
    SDK_ASSERT(sdk::asic::pd::asicpd_deparser_init() == SDK_RET_OK);
    SDK_ASSERT(sdk::asic::pd::asicpd_program_hbm_table_base_addr() == SDK_RET_OK);

    // g_hal_state_pd->qos_hbm_fifo_allocator_init();
    return HAL_RET_OK;
}

hal_ret_t
pd_pgm_def_entries (pd_func_args_t *pd_func_args)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_pgm_def_entries_args_t *args = pd_func_args->pd_pgm_def_entries;
    p4pd_def_cfg_t  p4pd_def_cfg;

    HAL_TRACE_DEBUG("Programming table default entries ...");
    p4pd_def_cfg.hal_cfg = args->hal_cfg;
    ret = p4pd_table_defaults_init(&p4pd_def_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program default entries, err: {}", ret);
    }

    return ret;
}

hal_ret_t
pd_pgm_def_p4plus_entries (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// free an element back to given PD slab specified by its id
//------------------------------------------------------------------------------
hal_ret_t
free_to_slab (hal_slab_t slab_id, void *elem)
{
    switch (slab_id) {
    case HAL_SLAB_LIF_PD:
        g_hal_state_pd->lif_pd_slab()->free(elem);
        break;

    case HAL_SLAB_VRF_PD:
        g_hal_state_pd->vrf_pd_slab()->free(elem);
        break;

    case HAL_SLAB_UPLINKIF_PD:
        g_hal_state_pd->uplinkif_pd_slab()->free(elem);
        break;

    case HAL_SLAB_ENICIF_PD:
        g_hal_state_pd->enicif_pd_slab()->free(elem);
        break;

    case HAL_SLAB_EP_PD:
        g_hal_state_pd->ep_pd_slab()->free(elem);
        break;

    case HAL_SLAB_GFT_EMP_PD:
        g_hal_state_pd->exact_match_profile_pd_slab()->free(elem);
        break;

    default:
        HAL_TRACE_ERR("Unknown slab id {}", slab_id);
        SDK_ASSERT(FALSE);
        return HAL_RET_INVALID_ARG;
        break;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// callback invoked by the timerwheel to release an object to its slab
//------------------------------------------------------------------------------
static inline void
pd_slab_delay_delete_cb (void *timer, hal_slab_t slab_id, void *elem)
{
    hal_ret_t    ret;

    if (slab_id < HAL_SLAB_PI_MAX) {
        ret = hal::free_to_slab(slab_id, elem);
    } else if (slab_id < HAL_SLAB_PD_MAX) {
        ret = hal::pd::free_to_slab(slab_id, elem);
    } else {
        HAL_TRACE_ERR("Unexpected slab id {}", slab_id);
        ret = HAL_RET_INVALID_ARG;
    }
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to release elem {} to slab id {}", elem, slab_id);
    }

    return;
}

//------------------------------------------------------------------------------
// wrapper function to delay delete slab elements
// NOTE: currently delay delete timeout is 2 seconds, it is expected that any
//       other threads using (a pointer to) this object should be done with this
//       object within this timeout or else this memory can be freed and
//       allocated for other objects and can result in corruptions. Hence, tune
//       this timeout, if needed
//------------------------------------------------------------------------------
hal_ret_t
delay_delete_to_slab (hal_slab_t slab_id, void *elem)
{
    void    *timer_ctxt;

    if (g_delay_delete && sdk::lib::periodic_thread_is_running()) {
        timer_ctxt =
            sdk::lib::timer_schedule(slab_id,
                                     TIME_MSECS_PER_SEC << 1, elem,
                                     (sdk::lib::twheel_cb_t)pd_slab_delay_delete_cb,
                                     false);
        if (!timer_ctxt) {
            return HAL_RET_ERR;
        }
    } else {
        pd_slab_delay_delete_cb(NULL, slab_id, elem);
    }
    return HAL_RET_OK;
}

// TODO: this doesn't belong in PD as this is not pipeline dependent
// move the code in iris to some common place so all HAL PDs can use
hal_ret_t
pd_clock_delta_comp (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// TODO: this doesn't belong in PD .. CPU tx/rx driver can't be in iris/gft, it
// is common to all PDs
hal_ret_t
pd_cpupkt_ctxt_alloc_init (pd_func_args_t *pd_func_args)
{
    pd_cpupkt_ctxt_alloc_init_args_t *args = pd_func_args->pd_cpupkt_ctxt_alloc_init;
    args->ctxt = NULL;
    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_register_tx_queue (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_register_rx_queue (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
