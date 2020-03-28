// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include "nic/hal/hal.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/plugins/cfg/rdma/rdma.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/p4/common/defines.h"
#include "nic/sdk/platform/capri/capri_hbm_rw.hpp"
#ifdef __x86_64__
#include "gen/hal/svc/rdma_svc_gen.hpp"
#endif

#ifdef __x86_64__
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
#endif

namespace hal {

RDMAManager *g_rdma_manager = nullptr;

RDMAManager *
rdma_manager()
{
    return g_rdma_manager;
}

#ifdef __x86_64__
RdmaServiceImpl    g_rdma_svc;

void
svc_reg (ServerBuilder *server_builder, hal::hal_feature_set_t feature_set)
{
    if (!server_builder) {
        return;
    }

    // register all "rdma" services
    HAL_TRACE_DEBUG("Registering gRPC rdma services ...");
    if (feature_set == hal::HAL_FEATURE_SET_IRIS ||
        feature_set == hal::HAL_FEATURE_SET_GFT) {
        server_builder->RegisterService(&g_rdma_svc);
    }
    HAL_TRACE_DEBUG("gRPC rdma services registered ...");
    return;
}
#endif

enum ionic_v1_stat_bits {
    IONIC_V1_STAT_TYPE_SHIFT    = 28,
    IONIC_V1_STAT_TYPE_NONE     = 0,
    IONIC_V1_STAT_TYPE_8        = 1,
    IONIC_V1_STAT_TYPE_LE16     = 2,
    IONIC_V1_STAT_TYPE_LE32     = 3,
    IONIC_V1_STAT_TYPE_LE64     = 4,
    IONIC_V1_STAT_TYPE_BE16     = 5,
    IONIC_V1_STAT_TYPE_BE32     = 6,
    IONIC_V1_STAT_TYPE_BE64     = 7,
};

static const struct {
    uint32_t        type_off;
    char            name[28];
} rdma_stats_hdrs[] = {
#define RDMA_STAT(_size, _off, _name) { \
    htobe32((IONIC_V1_STAT_TYPE_ ## _size <<            \
             IONIC_V1_STAT_TYPE_SHIFT) | (_off)),       \
    _name,                                              \
}
    //RDMA counters - Tx
    RDMA_STAT(LE64, LIF_STATS_TX_RDMA_UCAST_BYTES_OFFSET, "tx_rdma_ucast_bytes"),
    RDMA_STAT(LE64, LIF_STATS_TX_RDMA_UCAST_PACKETS_OFFSET, "tx_rdma_ucast_pkts"),
    RDMA_STAT(LE64, LIF_STATS_TX_RDMA_MCAST_BYTES_OFFSET, "tx_rdma_mcast_bytes"),
    RDMA_STAT(LE64, LIF_STATS_TX_RDMA_MCAST_PACKETS_OFFSET, "tx_rdma_mcast_pkts"),
    RDMA_STAT(LE64, LIF_STATS_TX_RDMA_CNP_PACKETS_OFFSET, "tx_rdma_cnp_pkts"),
    //RDMA counters - Rx
    RDMA_STAT(LE64, LIF_STATS_RX_RDMA_UCAST_BYTES_OFFSET, "rx_rdma_ucast_bytes"),
    RDMA_STAT(LE64, LIF_STATS_RX_RDMA_UCAST_PACKETS_OFFSET, "rx_rdma_ucast_pkts"),
    RDMA_STAT(LE64, LIF_STATS_RX_RDMA_MCAST_BYTES_OFFSET, "rx_rdma_mcast_bytes"),
    RDMA_STAT(LE64, LIF_STATS_RX_RDMA_MCAST_PACKETS_OFFSET, "rx_rdma_mcast_pkts"),
    RDMA_STAT(LE64, LIF_STATS_RX_RDMA_CNP_PACKETS_OFFSET, "rx_rdma_cnp_pkts"),
    RDMA_STAT(LE64, LIF_STATS_RX_RDMA_ECN_PACKETS_OFFSET, "rx_rdma_ecn_pkts"),
    //RDMA Requester error/debug counters
    RDMA_STAT(LE64, LIF_STATS_REQ_RX_PACKET_SEQ_ERR_OFFSET, "req_rx_pkt_seq_err"),
    RDMA_STAT(LE64, LIF_STATS_REQ_RX_RNR_RETRY_ERR_OFFSET, "req_rx_rnr_retry_err"),
    RDMA_STAT(LE64, LIF_STATS_REQ_RX_REMOTE_ACC_ERR_OFFSET, "req_rx_rmt_acc_err"),
    RDMA_STAT(LE64, LIF_STATS_REQ_RX_REMOTE_INV_REQ_ERR_OFFSET, "req_rx_rmt_req_err"),
    RDMA_STAT(LE64, LIF_STATS_REQ_RX_REMOTE_OPER_ERR_OFFSET, "req_rx_oper_err"),
    RDMA_STAT(LE64, LIF_STATS_REQ_RX_IMPLIED_NAK_SEQ_ERR_OFFSET, "req_rx_impl_nak_seq_err"),
    RDMA_STAT(LE64, LIF_STATS_REQ_RX_CQE_ERR_OFFSET, "req_rx_cqe_err"),
    RDMA_STAT(LE64, LIF_STATS_REQ_RX_CQE_FLUSH_ERR_OFFSET, "req_rx_cqe_flush"),
    RDMA_STAT(LE64, LIF_STATS_REQ_RX_DUPLICATE_RESPONSES_OFFSET, "req_rx_dup_response"),
    RDMA_STAT(LE64, LIF_STATS_REQ_RX_INVALID_PACKETS_OFFSET, "req_rx_inval_pkts"),
    RDMA_STAT(LE64, LIF_STATS_REQ_TX_LOCAL_ACCESS_ERR_OFFSET, "req_tx_loc_acc_err"),
    RDMA_STAT(LE64, LIF_STATS_REQ_TX_LOCAL_OPER_ERR_OFFSET, "req_tx_loc_oper_err"),
    RDMA_STAT(LE64, LIF_STATS_REQ_TX_MEMORY_MGMT_ERR_OFFSET, "req_tx_mem_mgmt_err"),
    RDMA_STAT(LE64, LIF_STATS_REQ_TX_RETRY_EXCEED_ERR_OFFSET, "req_tx_retry_excd_err"),
    RDMA_STAT(LE64, LIF_STATS_REQ_TX_LOCAL_SGL_INV_ERR_OFFSET, "req_tx_loc_sgl_inv_err"),
    //RDMA Responder error/debug counters
    RDMA_STAT(LE64, LIF_STATS_RESP_RX_DUP_REQUEST_OFFSET, "resp_rx_dup_request"),
    RDMA_STAT(LE64, LIF_STATS_RESP_RX_OUT_OF_BUFFER_OFFSET, "resp_rx_outof_buf"),
    RDMA_STAT(LE64, LIF_STATS_RESP_RX_OUT_OF_SEQ_OFFSET, "resp_rx_outouf_seq"),
    RDMA_STAT(LE64, LIF_STATS_RESP_RX_CQE_ERR_OFFSET, "resp_rx_cqe_err"),
    RDMA_STAT(LE64, LIF_STATS_RESP_RX_CQE_FLUSH_ERR_OFFSET, "resp_rx_cqe_flush"),
    RDMA_STAT(LE64, LIF_STATS_RESP_RX_LOCAL_LEN_ERR_OFFSET, "resp_rx_loc_len_err"),
    RDMA_STAT(LE64, LIF_STATS_RESP_RX_INV_REQUEST_OFFSET, "resp_rx_inval_request"),
    RDMA_STAT(LE64, LIF_STATS_RESP_RX_LOCAL_QP_OPER_ERR_OFFSET, "resp_rx_loc_oper_err"),
    RDMA_STAT(LE64, LIF_STATS_RESP_RX_OUT_OF_ATOMIC_RESOURCE_OFFSET, "resp_rx_outof_atomic"),
    RDMA_STAT(LE64, LIF_STATS_RESP_TX_PACKET_SEQ_ERR_OFFSET, "resp_tx_pkt_seq_err"),
    RDMA_STAT(LE64, LIF_STATS_RESP_TX_REMOTE_INV_REQ_ERR_OFFSET, "resp_tx_rmt_inval_req_err"),
    RDMA_STAT(LE64, LIF_STATS_RESP_TX_REMOTE_ACC_ERR_OFFSET, "resp_tx_rmt_acc_err"),
    RDMA_STAT(LE64, LIF_STATS_RESP_TX_REMOTE_OPER_ERR_OFFSET, "resp_tx_rmt_oper_err"),
    RDMA_STAT(LE64, LIF_STATS_RESP_TX_RNR_RETRY_ERR_OFFSET, "resp_tx_rnr_retry_err"),
    RDMA_STAT(LE64, LIF_STATS_RESP_TX_LOCAL_SGL_INV_ERR_OFFSET, "resp_tx_loc_sgl_inv_err"),
    RDMA_STAT(NONE, 0, "")
#undef RDMA_STAT
};

static void
rdma_stats_hdrs_init(void)
{
    pd::pd_hbm_write_mem_args_t args = {0};
    pd::pd_func_args_t pd_func_args = {0};

    args.addr = asicpd_get_mem_addr(CAPRI_HBM_REG_RDMA_STATS_HDRS);
    if (args.addr == INVALID_MEM_ADDRESS)
        return;

    SDK_ASSERT(RDMA_STATS_HDRS_SIZE ==
               asicpd_get_mem_size_kb(CAPRI_HBM_REG_RDMA_STATS_HDRS) * 1024);

    args.buf = (uint8_t *)&rdma_stats_hdrs;
    args.size = sizeof(rdma_stats_hdrs);

    if (args.size > RDMA_STATS_HDRS_SIZE) {
        HAL_TRACE_WARN("Limiting rdma stats headers");
        args.size = RDMA_STATS_HDRS_SIZE;
    }

    HAL_TRACE_DEBUG("{}: Writing rdma stats headers to addr {:#x} size {}\n",
                    __FUNCTION__, args.addr, args.size);

    pd_func_args.pd_hbm_write_mem = &args;
    pd::hal_pd_call(pd::PD_FUNC_ID_HBM_WRITE, &pd_func_args);
}

// initialization routine for rdma module
extern "C" hal_ret_t
rdma_init (hal::hal_cfg_t *hal_cfg)
{
#ifdef __x86_64__
    svc_reg((ServerBuilder *)hal_cfg->server_builder, hal_cfg->features);
#endif
    HAL_TRACE_DEBUG("{}: Entered\n", __FUNCTION__);
    g_rdma_manager = new RDMAManager();
    rdma_stats_hdrs_init();
    HAL_TRACE_DEBUG("{}: Leaving\n", __FUNCTION__);

    return HAL_RET_OK;
}

// cleanup routine for rdma module
extern "C" void
rdma_exit (void)
{
}

}    // namespace hal
