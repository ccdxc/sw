//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>

#include "include/sdk/mem.hpp"
#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"
#include "nic/sdk/asic/asic.hpp"
#include <nic/sdk/lib/pal/pal.hpp>

#include "ftlite.hpp"
#include "ftlite_table.hpp"
#include "ftlite_bucket.hpp"
#include "ftlite_utils.hpp"

using namespace ftlite::internal;

#define FTLITE_API_BEGIN(_name) {\
    FTLITE_TRACE_DEBUG("-- ftlite begin: --"); \
}

#define FTLITE_API_END(_status) {\
    FTLITE_TRACE_DEBUG("-- ftlite end: (r:%d) --", _status); \
}

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
    asic_cfg_t  asic_cfg;
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

    memset(&asic_cfg, 0, sizeof(asic_cfg_t));
    asic_cfg.cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    std::string mpart_json = asic_cfg.cfg_path + "/apollo/hbm_mem.json";
    asic_cfg.mempartition =
        sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    /* do capri_state_pd_init needed by sdk capri
     * csr init is done inside capri_state_pd_init */
    asicpd_state_pd_init(&asic_cfg);

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
init(init_params_t *ips) {
    return g_state.init(ips);
}

template<class T>
static sdk_ret_t insert_flow(meta_t* meta, T* info) {
    sdk_ret_t ret = SDK_RET_OK;
    auto &mtable = g_state.mtables[meta->ipv6]; // Main Table
    auto &htable = g_state.htables[meta->ipv6]; // Hint Table
    auto &ptable = meta->ptype ? htable : mtable; // Parent Table
    auto &ltable = meta->pslot ? htable : mtable; // Leaf Table
    uint32_t eindex = meta->hash.index;

    if (meta->pslot) { // Parent is valid.
        //info->pentry.swizzle();

        // Validate PARENT bucket
        auto &pbucket = ptable.bucket(meta->pindex);
        ret = pbucket.validate(&info->pentry, meta->pslot, meta->pindex);
        FTLITE_CHECK_AND_RETURN(ret);

        // Allocate new hint index
        ret = htable.alloc(eindex);
        FTLITE_CHECK_AND_RETURN(ret);

        FTLITE_TRACE_DEBUG("Allocated hint = %d", eindex);
    }

    // Validate LEAF bucket
    auto &bucket = ltable.bucket(eindex);
    ret = bucket.validate(&info->lentry);
    FTLITE_CHECK_AND_RETURN(ret);

    // Set the 'valid' bit in the main table bucket
    bucket.valid = 1;

    // Programming start
    info->lentry.entry_valid = 1;
    ret = ltable.write(meta->ipv6, &info->lentry, eindex);
    FTLITE_CHECK_AND_RETURN(ret);

    if (meta->pslot) {
        info->pentry.set_hint_hash(meta->pslot, eindex, meta->hash.msb);
        ret = ptable.write(meta->ipv6, &info->pentry, meta->pindex);
        FTLITE_CHECK_AND_RETURN(ret);
    }

    return SDK_RET_OK;
}

static sdk_ret_t
insert_flow_af(meta_t* meta, info_t* info) {
    auto ret = SDK_RET_OK;
    if (meta->ipv6) {
        ret = insert_flow<v6info_t>(meta, &info->v6);
    } else {
        ret = insert_flow<v4info_t>(meta, &info->v4);
    }
    return ret;
}

static sdk_ret_t
insert_session(insert_params_t *params) {
    return SDK_RET_OK;
}

template<class T> static void
trace_flow_(meta_t* meta, T* flow) {
    static char buff[4096];

    meta->tostr(buff, sizeof(buff));
    FTLITE_TRACE_DEBUG("Meta = [ %s ]", buff);

    flow->lentry.tostr(buff, sizeof(buff));
    FTLITE_TRACE_DEBUG("Leaf = [ %s ]", buff);

    if (meta->level) {
        flow->pentry.tostr(buff, sizeof(buff));
        FTLITE_TRACE_DEBUG("Parent = [ %s ]", buff);
    }
    return;
}

static void
trace_params_(insert_params_t *ips) {
    FTLITE_TRACE_DEBUG("IFLOW");
    if (ips->imeta.ipv6) {
        trace_flow_<v6info_t>(&ips->imeta, &ips->iflow.v6);
    } else {
        trace_flow_<v4info_t>(&ips->imeta, &ips->iflow.v4);
    }

    FTLITE_TRACE_DEBUG("RFLOW");
    if (ips->rmeta.ipv6) {
        trace_flow_<v6info_t>(&ips->rmeta, &ips->rflow.v6);
    } else {
        trace_flow_<v4info_t>(&ips->rmeta, &ips->rflow.v4);
    }

    return;
}

sdk_ret_t
insert(insert_params_t *ips) {
__label__ done;
    FTLITE_API_BEGIN();
    trace_params_(ips);

    auto ret = insert_session(ips);
    FTLITE_RET_CHECK(ret, "session");

    ret = insert_flow_af(&ips->imeta, &ips->iflow);
    FTLITE_RET_CHECK(ret, "iflow");

    ret = insert_flow_af(&ips->rmeta, &ips->rflow);
    FTLITE_RET_CHECK(ret, "rflow");

    FTLITE_API_END(ret);
done:
    return ret;
}

} // namespace ftlite
