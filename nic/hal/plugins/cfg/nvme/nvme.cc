//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <cstdlib>
#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nvme/nvme.hpp"
#include "nic/utils/host_mem/host_mem.hpp"
#include "nic/p4/common/defines.h"
#include "nic/hal/plugins/cfg/mcast/oif_list_api.hpp"
#include "nic/sdk/nvme/nvme_common.h"
#include "nic/include/nvme_dpath.h"
#include "nvme_global.hpp"
#include "nvme_sesscb.hpp"
#include "nvme_ns.hpp"
#include "nvme_sq.hpp"
#include "nvme_cq.hpp"


namespace hal {

typedef struct nvme_global_info_s {
    uint32_t max_ns;
    uint32_t cur_ns;
    uint32_t max_sess;
    uint32_t cur_sess;
    uint32_t max_cmd_context;
    uint32_t tx_max_pdu_context;
    uint32_t rx_max_pdu_context;
} nvme_global_info_t;

typedef struct nvme_lif_info_s {
    uint32_t max_ns;
    uint32_t max_sess;
    uint32_t cur_sess;
    uint32_t sess_start;
    uint32_t max_cq;
    uint32_t max_sq;
    uint32_t log_host_page_size;
    uint32_t ns_start;
} nvme_lif_info_t;

typedef struct nvme_ns_info_s {
    uint32_t sess_start;
    uint32_t max_sess;
    uint32_t cur_sess;
    uint32_t key_index;
    uint32_t sec_key_index;
    uint16_t log_lba_size;
} nvme_ns_info_t;

static nvme_global_info_t g_nvme_global_info;
static nvme_lif_info_t g_nvme_lif_info[MAX_LIFS];
static nvme_ns_info_t *g_nvme_ns_info = NULL;


//NVMEManager *g_nvme_manager = nullptr;
extern lif_mgr *lif_manager();

NVMEManager::NVMEManager() {
}

static inline uint8_t *
memrev (uint8_t *block, size_t elnum)
{
     uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
     return block;
}

uint32_t
roundup_to_pow_2(uint32_t x)
{
    uint32_t power = 1;

    if (x == 1)
        return (power << 1);

    while(power < x)
        power*=2;
    return power;
}

hal_ret_t
nvme_enable (NvmeEnableRequest& spec, NvmeEnableResponse *rsp)
{
    int32_t            max_ns;
    int32_t            max_sess;
    int32_t            max_cmd_context;
    int32_t            tx_max_pdu_context;
    int32_t            rx_max_pdu_context;
    hal_ret_t          ret;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
 
    HAL_TRACE_DEBUG("NVME Enable Request");

    max_ns  = spec.max_ns();
    max_sess  = spec.max_sess();
    max_cmd_context  = spec.max_cmd_context();
    tx_max_pdu_context = spec.tx_max_pdu_context();
    rx_max_pdu_context = spec.rx_max_pdu_context();

    HAL_TRACE_DEBUG("max_ns: {}, max_sess: {}, max_cmd_context: {}, "
                    "tx_max_pdu_context: {}, rx_max_pdu_context: {}\n",
                     max_ns, max_sess, max_cmd_context, 
                     tx_max_pdu_context, rx_max_pdu_context);

    memset(&g_nvme_global_info, 0, sizeof(g_nvme_global_info));

    g_nvme_global_info.max_ns = max_ns;
    g_nvme_global_info.max_sess = max_sess;
    g_nvme_global_info.max_cmd_context = max_cmd_context;
    g_nvme_global_info.tx_max_pdu_context = tx_max_pdu_context;
    g_nvme_global_info.rx_max_pdu_context = rx_max_pdu_context;

    g_nvme_ns_info = (nvme_ns_info_t *)malloc(sizeof(nvme_ns_info_t) * max_ns);
    SDK_ASSERT(g_nvme_ns_info != NULL);
    memset(g_nvme_ns_info, 0, sizeof(nvme_ns_info_t) * max_ns);

    ret = nvme_global_create(MAX_LIFS, max_ns, max_sess, 
                             max_cmd_context, tx_max_pdu_context,
                             rx_max_pdu_context, rsp);
    
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed(ret: {:#x}) to create PD Nvme Global context",
                      ret);
        return HAL_RET_ERR;
    }

    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}

hal_ret_t
nvme_lif_init (intf::LifSpec& spec, uint32_t lif)
{
    uint32_t            max_ns, max_sess;
    uint32_t            max_cqs, max_sqs;
    nvme_lif_info_t     *nvme_lif_info_p;

    HAL_TRACE_DEBUG("cur_ns: {}, max_ns: {}, cur_sess: {}, max_sess: {}\n",
                    g_nvme_global_info.cur_ns, g_nvme_global_info.max_ns,
                    g_nvme_global_info.cur_sess, g_nvme_global_info.max_sess);

    HAL_TRACE_DEBUG("lif: {} spec_lif: {}", lif, spec.key_or_handle().lif_id());

    //SDK_ASSERT(lif == spec.key_or_handle().lif_id());
    SDK_ASSERT(lif < MAX_LIFS);

    max_ns = spec.nvme_max_ns();
    max_sess = spec.nvme_max_sess();

    HAL_TRACE_DEBUG("LIF: {}, max_ns: {}, max_sess: {}",
                    lif, max_ns, max_sess);

    SDK_ASSERT((g_nvme_global_info.cur_ns + max_ns) < g_nvme_global_info.max_ns);
    SDK_ASSERT((g_nvme_global_info.cur_sess + max_sess) < g_nvme_global_info.max_sess);

    nvme_lif_info_p = &g_nvme_lif_info[lif];

    memset(nvme_lif_info_p, 0, sizeof(nvme_lif_info_t));

    // LIFQState *qstate = lif_manager()->GetLIFQState(lif);
    lif_qstate_t *qstate = lif_manager()->get_lif_qstate(lif);
    if (qstate == nullptr)
        return HAL_RET_ERR;

    max_cqs  = qstate->type[NVME_QTYPE_CQ].num_queues;
    max_sqs  = qstate->type[NVME_QTYPE_SQ].num_queues;

    HAL_TRACE_DEBUG("LIF {}, max_CQ: {}, max_SQ: {}",
                    lif, max_cqs, max_sqs);

    nvme_lif_info_p->ns_start = g_nvme_global_info.cur_ns;
    g_nvme_global_info.cur_ns += max_ns;
    
    nvme_lif_info_p->sess_start = g_nvme_global_info.cur_sess;
    g_nvme_global_info.cur_sess += max_sess;
    
    nvme_lif_info_p->max_ns = max_ns;
    nvme_lif_info_p->max_sess = max_sess;
    nvme_lif_info_p->cur_sess = 0;

    HAL_TRACE_DEBUG("Lif {} max_cqs: {} log_max_cq_entries: {}",
                    lif, 
                    max_cqs, log2(roundup_to_pow_2(max_cqs)));
    nvme_lif_info_p->max_cq = max_cqs;

    HAL_TRACE_DEBUG("Lif {} max_sqs: {} log_max_sq_entries: {}",
                    lif, 
                    max_sqs, log2(roundup_to_pow_2(max_sqs)));
    nvme_lif_info_p->max_sq = max_sqs;

    SDK_ASSERT((spec.nvme_host_page_size() & (spec.nvme_host_page_size() - 1)) == 0);
    nvme_lif_info_p->log_host_page_size = log2(spec.nvme_host_page_size());

    HAL_TRACE_DEBUG("Lif: {}: max_sq: {}, "
                    "max_cq: {}, "
                    "max_ns: {}, ns_start: {}, "
                    "max_sess: {}, sess_start: {} "
                    "log_host_page_size: {}",
                    lif,
                    nvme_lif_info_p->max_sq, 
                    nvme_lif_info_p->max_cq, 
                    nvme_lif_info_p->max_ns, 
                    nvme_lif_info_p->ns_start,
                    nvme_lif_info_p->max_sess, 
                    nvme_lif_info_p->sess_start, 
                    nvme_lif_info_p->log_host_page_size);

    HAL_TRACE_DEBUG("Lif: {}: LIF Init successful\n", lif);

    return HAL_RET_OK;
}

hal_ret_t
nvme_sq_create (NvmeSqSpec& spec, NvmeSqResponse *rsp)
{
    hal_ret_t    ret = HAL_RET_OK;
    uint32_t     lif = spec.hw_lif_id();
    uint32_t     num_sq_wqes, sqwqe_size;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF: NVME SQ Create for lif {}", lif);

    HAL_TRACE_DEBUG("Inputs: sq_num: {} sq_wqe_size: {} num_sq_wqes: {} "
                    "base_addr: {} cq_num: {} lif_ns_start: {}",
                    spec.sq_num(),
                    spec.sq_wqe_size(), spec.num_sq_wqes(), 
                    spec.base_addr(), spec.cq_num(),
                    g_nvme_lif_info[lif].ns_start);

    SDK_ASSERT(lif < MAX_LIFS);
    SDK_ASSERT(spec.sq_num() < g_nvme_lif_info[lif].max_sq);
    SDK_ASSERT(spec.cq_num() < g_nvme_lif_info[lif].max_cq);

    sqwqe_size = roundup_to_pow_2(spec.sq_wqe_size());
    num_sq_wqes = roundup_to_pow_2(spec.num_sq_wqes());

    HAL_TRACE_DEBUG("sqwqe_size: {} num_sqwqes: {}", sqwqe_size, num_sq_wqes);

    ret = nvme_sq_create(lif, spec.sq_num(), log2(sqwqe_size), log2(num_sq_wqes),
                         g_nvme_lif_info[lif].log_host_page_size,
                         spec.cq_num(), g_nvme_lif_info[lif].ns_start, 
                         spec.base_addr(), rsp);

    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed(ret: {}) to create nvme_sq for lif: {} sq_id: {}",
                      ret, lif, spec.sq_num());
        return HAL_RET_ERR;
    }

    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}

hal_ret_t
nvme_cq_create (NvmeCqSpec& spec, NvmeCqResponse *rsp)
{
    hal_ret_t    ret = HAL_RET_OK;
    uint32_t     lif = spec.hw_lif_id();
    uint32_t     num_cq_wqes, cqwqe_size;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:NVME CQ Create for lif {}", lif);


    HAL_TRACE_DEBUG("Inputs: cq_num: {} cq_wqe_size: {} num_cq_wqes: {} "
                    "base_addr: {} int_num: {}",
                    spec.cq_num(),
                    spec.cq_wqe_size(), spec.num_cq_wqes(), 
                    spec.base_addr(), spec.int_num());

    SDK_ASSERT(lif < MAX_LIFS);
    SDK_ASSERT(spec.cq_num() < g_nvme_lif_info[lif].max_cq);

    cqwqe_size = roundup_to_pow_2(spec.cq_wqe_size());
    num_cq_wqes = roundup_to_pow_2(spec.num_cq_wqes());

    HAL_TRACE_DEBUG("cqwqe_size: {} num_cq_wqes: {}", cqwqe_size, num_cq_wqes);

    ret = nvme_cq_create(lif, spec.cq_num(), log2(cqwqe_size), log2(num_cq_wqes),
                         spec.int_num(), spec.base_addr(), rsp);

    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed(ret: {}) to create nvme_cq for lif: {} cq_id: {}",
                      ret, lif, spec.cq_num());
        return HAL_RET_ERR;
    }

    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}


hal_ret_t
nvme_ns_create (NvmeNsSpec& spec, NvmeNsResponse *rsp)
{
    hal_ret_t    ret = HAL_RET_OK;
    uint32_t     lif = spec.hw_lif_id();
    uint32_t     g_nsid;
    nvme_ns_info_t *ns_info_p;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF: NVME NS Create for lif {}", lif);


    HAL_TRACE_DEBUG("Inputs: nsid: {} backend_nsid: {} max_sessions: {} "
                    "size: {} lba_size: {} key_index: {} sec_key_index: {}",
                    spec.nsid(),
                    spec.backend_nsid(), spec.max_sess(), 
                    spec.size(), spec.lba_size(),
                    spec.key_index(), spec.sec_key_index());

    SDK_ASSERT(lif < MAX_LIFS);
    SDK_ASSERT(spec.nsid() != 0);
    SDK_ASSERT(spec.nsid() <= g_nvme_lif_info[lif].max_ns);
    SDK_ASSERT((spec.lba_size() & (spec.lba_size() - 1)) == 0); //power of 2 check
    SDK_ASSERT(g_nvme_lif_info[lif].cur_sess + spec.max_sess() <= g_nvme_lif_info[lif].max_sess);

    //update global ns info
    g_nsid = g_nvme_lif_info[lif].ns_start + spec.nsid() - 1;
    SDK_ASSERT(g_nsid <= g_nvme_global_info.max_ns);

    ns_info_p = &g_nvme_ns_info[g_nsid];
    ns_info_p->sess_start = g_nvme_lif_info[lif].sess_start + g_nvme_lif_info[lif].cur_sess;
    ns_info_p->max_sess = spec.max_sess();
    ns_info_p->cur_sess = 0;
    ns_info_p->key_index = spec.key_index();
    ns_info_p->sec_key_index = spec.sec_key_index();
    ns_info_p->log_lba_size = log2(spec.lba_size());

    g_nvme_lif_info[lif].cur_sess += spec.max_sess();

    HAL_TRACE_DEBUG("ns->sess_start: {}, lif->curr_sess: {},  key_index: {} sec_key_index: {}",
                    ns_info_p->sess_start, 
                    g_nvme_lif_info[lif].cur_sess,
                    ns_info_p->key_index, ns_info_p->sec_key_index);

    ret = nvme_ns_create(lif, spec.nsid(), g_nsid, spec.backend_nsid(), 
                         spec.max_sess(), spec.size(), spec.lba_size(),
                         spec.key_index(), spec.sec_key_index(), 
                         ns_info_p->sess_start, rsp);

    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed(ret: {}) to create nvme_ns for lif: {} lif_nsid: {} g_nsid: {}",
                      ret, lif, spec.nsid(), g_nsid);
        return HAL_RET_ERR;
    }

    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (ret);
}

hal_ret_t
nvme_sess_create (NvmeSessSpec& spec, NvmeSessResponse *rsp)
{
    uint32_t      lif = spec.hw_lif_id();
    uint32_t      ns_sess_id; //NS local
    uint32_t      lif_sess_id; //LIF local
    uint32_t      g_sess_id; //Global
    uint32_t      sesq_qid;
    uint32_t      serq_qid;
    hal_ret_t     ret;
    uint32_t      g_nsid;
    nvme_ns_info_t *ns_info_p;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF: NVME Sess Create for lif {}", lif);


    HAL_TRACE_DEBUG("Inputs: nsid: {}",
                    spec.nsid())

    SDK_ASSERT(lif < MAX_LIFS);
    SDK_ASSERT(spec.nsid() != 0);
    SDK_ASSERT(spec.nsid() <= g_nvme_lif_info[lif].max_ns);

    g_nsid = g_nvme_lif_info[lif].ns_start + spec.nsid() - 1;
    SDK_ASSERT(g_nsid <= g_nvme_global_info.max_ns);

    ns_info_p = &g_nvme_ns_info[g_nsid];

    SDK_ASSERT(ns_info_p->cur_sess < (ns_info_p->max_sess - 1));

    //Get tcp qid associated with the flow
    proxy_flow_info_t*  pfi = NULL;
    flow_key_t          flow_key = {0};
    vrf_id_t            tid = 0;

    tid = spec.vrf_key_handle().vrf_id();
    extract_flow_key_from_spec(tid, &flow_key, spec.flow_key());

    HAL_TRACE_DEBUG("vrf_id: {}, tid: {}, flow_key: {}",
                    spec.vrf_key_handle().vrf_id(),
                    tid, flow_key);

    pfi = proxy_get_flow_info(types::PROXY_TYPE_TCP, flow_key);
    if(!pfi) {
        HAL_TRACE_ERR("flow info not found for the flow {}", flow_key);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_PROXY_NOT_FOUND;
    }

    SDK_ASSERT(pfi != NULL);
    SDK_ASSERT(pfi->proxy != NULL);
    SDK_ASSERT(pfi->proxy->type == types::PROXY_TYPE_TCP);

    serq_qid = sesq_qid = pfi->qid2;

    HAL_TRACE_DEBUG("TCP Flow LIF: {}, QType: {}, QID1: {}, QID2: {}",
                    pfi->proxy->meta->lif_info[0].lif_id,
                    pfi->proxy->meta->lif_info[0].qtype_info[0].qtype_val,
                    pfi->qid1, pfi->qid2);

    //global session id
    ns_sess_id = ns_info_p->cur_sess++;
    g_sess_id = ns_info_p->sess_start + ns_sess_id;
    SDK_ASSERT(g_sess_id >= g_nvme_lif_info[lif].sess_start);
    lif_sess_id = g_sess_id - g_nvme_lif_info[lif].sess_start;
    SDK_ASSERT(g_sess_id < g_nvme_global_info.max_sess);

    HAL_TRACE_DEBUG("NS Local session id: {}, LIF local session id: {}, Global session id: {}",
                    ns_sess_id, lif_sess_id, g_sess_id);

    ret = nvme_sesscb_create(lif, g_nsid, g_sess_id, lif_sess_id, ns_sess_id, sesq_qid, serq_qid, rsp);

    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed(ret: {}) to create nvme_sesscb for lif: {} sess_id: {}",
                      ret, lif, g_sess_id);
        return HAL_RET_ERR;
    }

    rsp->set_api_status(types::API_STATUS_OK);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");

    return (HAL_RET_OK);
}


}    // namespace hal
