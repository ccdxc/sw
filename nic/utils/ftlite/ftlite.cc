//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>

#include "include/sdk/mem.hpp"
#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"
#include <nic/sdk/lib/pal/pal.hpp>

#include "ftlite.hpp"
#include "ftlite_table.hpp"
#include "ftlite_bucket.hpp"
#include "ftlite_utils.hpp"

using namespace ftlite::internal;

#if 0
extern "C" {


int ftlite_insert(ftlite::key_params_t *hdr);
int ftlite_init(void);
int initialize_pds(void);

int
initialize_pds(void)
{
    pal_ret_t    pal_ret;
    p4pd_error_t p4pd_ret;
    capri_cfg_t  capri_cfg;
    sdk_ret_t    ret;

    p4pd_cfg_t p4pd_cfg = {
        .table_map_cfg_file  = "apollo/capri_p4_table_map.json",
        .p4pd_pgm_name       = "apollo_p4",
        .p4pd_rxdma_pgm_name = "apollo_rxdma",
        .p4pd_txdma_pgm_name = "apollo_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
    p4pd_cfg_t p4pd_rxdma_cfg = {
        .table_map_cfg_file  = "apollo/capri_rxdma_table_map.json",
        .p4pd_pgm_name       = "apollo_p4",
        .p4pd_rxdma_pgm_name = "apollo_rxdma",
        .p4pd_txdma_pgm_name = "apollo_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };
    p4pd_cfg_t p4pd_txdma_cfg = {
        .table_map_cfg_file  = "apollo/capri_txdma_table_map.json",
        .p4pd_pgm_name       = "apollo_p4",
        .p4pd_rxdma_pgm_name = "apollo_rxdma",
        .p4pd_txdma_pgm_name = "apollo_txdma",
        .cfg_path = std::getenv("HAL_CONFIG_PATH")
    };

    /* initialize PAL */
    pal_ret = sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW);
    SDK_ASSERT(pal_ret == sdk::lib::PAL_RET_OK);

    memset(&capri_cfg, 0, sizeof(capri_cfg_t));
    capri_cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    std::string mpart_json = capri_cfg.cfg_path + "/apollo/hbm_mem.json";
    capri_cfg.mempartition =
        sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    /* do capri_state_pd_init needed by sdk capri
     * csr init is done inside capri_state_pd_init */
    sdk::platform::capri::capri_state_pd_init(&capri_cfg);

    /* do apollo specific initialization */
    p4pd_ret = p4pd_init(&p4pd_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    p4pd_ret = p4pluspd_rxdma_init(&p4pd_rxdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    p4pd_ret = p4pluspd_txdma_init(&p4pd_txdma_cfg);
    SDK_ASSERT(p4pd_ret == P4PD_SUCCESS);

    ret = sdk::asic::pd::asicpd_program_hbm_table_base_addr();
    SDK_ASSERT(ret == SDK_RET_OK);

    capri::capri_table_csr_cache_inval_init();

    return 0;
}

int
ftlite_init(void)
{
    ftlite::init_params_t params = {0};

    initialize_pds();

    params.ipv6_main_table_id = 5; //P4TBL_ID_FLOW;
    params.ipv6_num_hints = 5;
    auto r = ftlite::init(&params);
    SDK_ASSERT(r == SDK_RET_OK);

    return 0;
}

}
#endif

namespace ftlite {

static state_t g_state;

sdk_ret_t
init(init_params_t *params) {
__label__ done;
    sdk_ret_t ret = SDK_RET_OK;

    g_state.lock();

    if (params->ipv4_main_table_id) {
        auto &mtable = g_state.main_tables[IPAF_IPV4];
        ret = mtable.init(params->ipv4_main_table_id);
        if (ret != SDK_RET_OK) {
            goto done;
        }

        auto &htable = g_state.hint_tables[IPAF_IPV4];
        ret = htable.init(mtable.oflow_table_id());
        if (ret != SDK_RET_OK) {
            goto done;
        }
    }

    if (params->ipv6_main_table_id) {
        auto &mtable = g_state.main_tables[IPAF_IPV6];
        ret = mtable.init(params->ipv6_main_table_id);
        if (ret != SDK_RET_OK) {
            goto done;
        }

        auto &htable = g_state.hint_tables[IPAF_IPV6];
        ret = htable.init(mtable.oflow_table_id());
        if (ret != SDK_RET_OK) {
            goto done;
        }
    }

done:
    g_state.unlock();
    return SDK_RET_OK;
}

template<class T>
static sdk_ret_t insert_flow(T& flow) {
    sdk_ret_t ret = SDK_RET_OK;
    auto &mtable = g_state.main_tables[flow.tblmeta.ipaf]; // Main Table
    auto &htable = g_state.hint_tables[flow.tblmeta.ipaf]; // Hint Table
    auto &ptable = flow.tblmeta.ptype ? htable : mtable; // Parent Table
    auto &ltable = flow.tblmeta.pslot ? htable : mtable; // Leaf Table
    uint32_t eindex = flow.tblmeta.hash.index;

    if (flow.tblmeta.pslot) { // Parent is valid.
        //flow.parent.entry.swizzle();

        // Validate PARENT bucket
        auto &pbucket = ptable.bucket(flow.tblmeta.pindex);
        ret = pbucket.validate(&flow.parent.entry,
                               flow.tblmeta.pslot, flow.tblmeta.pindex);
        FTLITE_CHECK_AND_RETURN(ret);

        // Allocate new hint index
        ret = htable.alloc(eindex);
        FTLITE_CHECK_AND_RETURN(ret);
    }
    
    // Validate LEAF bucket
    auto &bucket = ltable.bucket(eindex);
    ret = bucket.validate(&flow.leaf.entry);
    FTLITE_CHECK_AND_RETURN(ret);
        
    // Set the 'valid' bit in the main table bucket
    bucket.valid = 1;
    
    // Programming start
    flow.leaf.entry.entry_valid = 1;
    ret = ltable.write(&flow.leaf.entry, eindex);
    FTLITE_CHECK_AND_RETURN(ret);
    
    if (flow.tblmeta.pslot) {
        flow.parent.entry.set_hint_hash(flow.tblmeta.pslot,
                                        eindex, flow.tblmeta.hash.msb);
        ret = ptable.write(&flow.parent.entry, flow.tblmeta.pindex);
        FTLITE_CHECK_AND_RETURN(ret);
    }
                                         
    return SDK_RET_OK;
}

static sdk_ret_t
insert_flow_ipaf(flow_meta_t& meta) {
    auto ret = SDK_RET_OK;
    if (meta.ipv4meta.tblmeta.ipaf == IPAF_IPV4) {
        ret = insert_flow<ipv4_flow_meta_t>(meta.ipv4meta);
    } else {
        ret = insert_flow<ipv6_flow_meta_t>(meta.ipv6meta);
    }
    return ret;
}

static sdk_ret_t
insert_session(insert_params_t *params) {
    return SDK_RET_OK;
}

sdk_ret_t
insert(insert_params_t *params) {
    auto ret = insert_session(params);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    
    ret = insert_flow_ipaf(params->iflow);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = insert_flow_ipaf(params->rflow);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    
    return SDK_RET_OK;
}

} // namespace ftlite
