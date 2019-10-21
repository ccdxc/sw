#include "nic/fte/fte.hpp"
#include "nic/fte/fte_impl.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/include/pd_api.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"

#ifdef SIM
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#endif

namespace fte {
namespace impl {

void cfg_db_open()
{
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
}

void cfg_db_close()
{
    hal::hal_cfg_db_close();    
}

hal::pd::cpupkt_ctxt_t *
cpupkt_ctxt_alloc_init(uint32_t qid)
{
    hal_ret_t ret;
    hal::pd::pd_cpupkt_ctxt_alloc_init_args_t args;
    hal::pd::pd_cpupkt_register_rx_queue_args_t rx_args;
    hal::pd::pd_cpupkt_register_tx_queue_args_t tx_args;
    hal::pd::pd_func_args_t          pd_func_args = {0};

    pd_func_args.pd_cpupkt_ctxt_alloc_init = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_ALLOC_INIT, &pd_func_args);
    SDK_ASSERT(ret == HAL_RET_OK);

    rx_args.ctxt = args.ctxt;
    rx_args.type = types::WRING_TYPE_ASCQ;

    rx_args.queue_id = qid;
    pd_func_args.pd_cpupkt_register_rx_queue = &rx_args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_REG_RXQ, &pd_func_args);
    SDK_ASSERT(ret == HAL_RET_OK);

    rx_args.type = types::WRING_TYPE_ARQRX;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_REG_RXQ, &pd_func_args);
    SDK_ASSERT(ret == HAL_RET_OK);

    tx_args.ctxt = args.ctxt;
    tx_args.type = types::WRING_TYPE_ASQ;
    tx_args.queue_id = qid;
    pd_func_args.pd_cpupkt_register_tx_queue = &tx_args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_REG_TXQ, &pd_func_args);
    SDK_ASSERT(ret == HAL_RET_OK);

    return args.ctxt;
}

hal_ret_t cpupkt_poll_receive(hal::pd::cpupkt_ctxt_t *ctx,
                              cpu_rxhdr_t **cpu_rxhdr,
                              uint8_t **pkt, size_t *pkt_len, bool *copied_pkt)
{
    hal::pd::pd_cpupkt_poll_receive_args_t args;
    hal::pd::pd_func_args_t pd_func_args = {0};
    args.ctxt = ctx;
    args.flow_miss_hdr = cpu_rxhdr;
    args.data = pkt;
    args.data_len = pkt_len;
    args.copied_pkt = copied_pkt;
    pd_func_args.pd_cpupkt_poll_receive = &args;

    return hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_POLL_RECV, &pd_func_args);

}

hal_ret_t cpupkt_poll_receive_new (hal::pd::cpupkt_ctxt_t *ctx,
				   hal::pd::cpupkt_pkt_batch_t *pkt_batch)
{
    hal::pd::pd_cpupkt_poll_receive_new_args_t args;
    hal::pd::pd_func_args_t pd_func_args = {0};

    args.ctxt = ctx;
    args.pkt_batch = pkt_batch;
    pd_func_args.pd_cpupkt_poll_receive_new = &args;

    return hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_POLL_RECV_NEW, &pd_func_args);
}

hal_ret_t cpupkt_send(hal::pd::cpupkt_ctxt_t *ctx,
                   uint32_t qid,
                   hal::pd::cpu_to_p4plus_header_t* cpu_header,
                   hal::pd::p4plus_to_p4_header_t* p4plus_header,
                   uint8_t* pkt, size_t pkt_len)
{
    hal::pd::pd_cpupkt_send_args_t args;
    hal::pd::pd_func_args_t pd_func_args = {0};
    args.ctxt = ctx;
    args.type = types::WRING_TYPE_ASQ;
    args.queue_id = qid;
    args.cpu_header = cpu_header;
    args.p4_header = p4plus_header;
    args.data = pkt;
    args.data_len = pkt_len;
    args.dest_lif = HAL_LIF_CPU;
    args.qtype = CPU_ASQ_QTYPE;
    args.qid = qid;
    args.ring_number = CPU_SCHED_RING_ASQ;

    pd_func_args.pd_cpupkt_send = &args;
    return hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_SEND, &pd_func_args);
}

//------------------------------------------------------------------------------
// Process a pkt from TLS asym pending request queue
//------------------------------------------------------------------------------
void process_pending_queues()
{
#ifdef SIM
    hal::proxy::tls_poll_asym_pend_req_q();
#endif
}

//------------------------------------------------------------------------------
// Allocate and Initialize TCP Rings Ctx
//------------------------------------------------------------------------------
void *init_tcp_rings_ctxt(uint8_t fte_id, void *arm_ctx)
{
    void * ctxt = NULL;
#ifdef SIM
    ctxt = hal::proxy::tcp_rings_ctxt_init(fte_id, arm_ctx);
#endif
    return ctxt;
}

//------------------------------------------------------------------------------
// Process msgs from TCP ACTL queue
//------------------------------------------------------------------------------
void process_tcp_queues(void *tcp_ctx)
{
#ifdef SIM
    hal::proxy::tcp_rings_poll(tcp_ctx);
#endif
}

}
}
