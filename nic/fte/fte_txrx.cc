#include "nic/fte/fte.hpp"
#include "nic/fte/fte_softq.hpp"
#include "nic/fte/fte_flow.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/p4/iris/include/defines.h"
#include "nic/include/cpupkt_api.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"

namespace fte {

static const uint16_t MAX_SOFTQ_SLOTS(1024);

static bool fte_disabled_;

void disable_fte()
{
    fte_disabled_ = true;
}

//------------------------------------------------------------------------------
// FTE Instance
//------------------------------------------------------------------------------
class inst_t {
public:
    inst_t(uint8_t fte_id);
    void start(void);
    hal_ret_t asq_send(hal::pd::cpu_to_p4plus_header_t* cpu_header,
                       hal::pd::p4plus_to_p4_header_t* p4plus_header,
                       uint8_t* pkt, size_t pkt_len);
    hal_ret_t softq_enqueue(softq_fn_t fn, void *data);
    uint8_t get_id() const {return id_;};
    ipc_logger *get_ipc_logger() const { return logger_; }
private:
    uint8_t                 id_;
    hal::pd::cpupkt_ctxt_t *arm_ctx_;
    mpscq_t                *softq_;

    ctx_t                  *ctx_;
    feature_state_t        *feature_state_;
    size_t                  feature_state_size_;
    uint16_t                num_features_;
    flow_t                 *iflow_;
    flow_t                 *rflow_;
    ipc_logger             *logger_;

    void process_arq();
    void process_softq();
    void process_tls_pendq();
    void ctx_mem_init();
};

//------------------------------------------------------------------------------
// Per thread FTE instances
//------------------------------------------------------------------------------
static inst_t *g_inst_list[hal::MAX_FTE_THREADS];

//------------------------------------------------------------------------------
// FTE instance of current thread
//------------------------------------------------------------------------------
thread_local inst_t *g_inst;

//------------------------------------------------------------------------------
// FTE main pkt loop
// Creates FTE instance and starts it
//------------------------------------------------------------------------------
void
fte_start(uint8_t fte_id)
{
    HAL_ASSERT(g_inst == NULL);
    HAL_ASSERT(fte_id < hal::MAX_FTE_THREADS);
    HAL_ASSERT(g_inst_list[fte_id] == NULL);

    g_inst = g_inst_list[fte_id] = new inst_t(fte_id);
    g_inst->start();
}

//------------------------------------------------------------------------------
// Send a packet on ASQ
// ***Should be called from FTE thread***
//------------------------------------------------------------------------------
hal_ret_t
fte_asq_send(hal::pd::cpu_to_p4plus_header_t* cpu_header,
             hal::pd::p4plus_to_p4_header_t* p4plus_header,
             uint8_t* pkt, size_t pkt_len)
{
    HAL_ASSERT_RETURN(g_inst, HAL_RET_INVALID_ARG);
    return g_inst->asq_send(cpu_header, p4plus_header, pkt, pkt_len);
}

//------------------------------------------------------------------------------
// Get fte_id
// Should be called from FTE thread
//------------------------------------------------------------------------------
uint8_t
fte_id()
{
    if (fte_disabled_) {
        return 0;
    }

    HAL_ASSERT_RETURN(g_inst, HAL_RET_INVALID_ARG);
    return g_inst->get_id();
}

//------------------------------------------------------------------------------
// Get ipc_logger
// Should be called from FTE thread
//------------------------------------------------------------------------------
ipc_logger*
get_current_ipc_logger_inst()
{
    if (fte_disabled_) {
        return 0;
    }

    HAL_ASSERT_RETURN(g_inst, NULL);
    return g_inst->get_ipc_logger();
}


//------------------------------------------------------------------------------
// Asynchronouly executes the fn in the specified fte thread,
// If the softq is full, it blocks until a slot is empty.
// Should be called from non FTE thread
//-----------------------------------------------------------------------------
hal_ret_t
fte_softq_enqueue(uint8_t fte_id, softq_fn_t fn, void *data)
{
    if (fte_disabled_) {
        // call the function directly
        fn(data);
    }

    HAL_ASSERT_RETURN(g_inst == NULL, HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(fte_id < hal::MAX_FTE_THREADS, HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(fn, HAL_RET_INVALID_ARG);

    inst_t *inst = g_inst_list[fte_id];

    if (inst == NULL) {
        HAL_TRACE_ERR("fte: fte.{} is not initializd", fte_id);
        return HAL_RET_ERR;
    }

    return inst->softq_enqueue(fn, data);
}

//------------------------------------------------------------------------
// Executes the fn in the specified fte thread and blocks until the
// the function is executed by the fte thread.
//------------------------------------------------------------------------
hal_ret_t
fte_execute(uint8_t fte_id, softq_fn_t fn, void *data)
{
    struct fn_ctx_t {
        volatile std::atomic<bool> done;
        softq_fn_t user_fn;
        void *user_data;
    } ctx;

    ctx.user_fn = fn;
    ctx.user_data = data;
    ctx.done.store(false, std::memory_order_release);

    hal_ret_t ret = fte_softq_enqueue(fte_id, [](void *data) {
            fn_ctx_t *ctx = (fn_ctx_t *) data;
            ctx->user_fn(ctx->user_data);
            ctx->done.store(true, std::memory_order_release); },
        &ctx);

    if (ret != HAL_RET_OK) {
        return ret;
    }

    // try indefinatly until done
    sdk::lib::thread *curr_thread = hal::hal_get_current_thread();
    while(ctx.done.load(std::memory_order_acquire) == false) {
        if (curr_thread->can_yield()) {
            pthread_yield();
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// byte array to hex string for logging
//------------------------------------------------------------------------------
std::string hex_str(const uint8_t *buf, size_t sz)
{
    std::ostringstream result;

    for(size_t i = 0; i < sz; i+=8) {
        result << " 0x";
        for (size_t j = i ; j < sz && j < i+8; j++) {
            result << std::setw(2) << std::setfill('0') << std::hex << (int)buf[j];
        }
    }

    return result.str();
}

hal::pd::cpupkt_ctxt_t *
fte_cpupkt_ctxt_alloc_init()
{
    hal::pd::pd_cpupkt_ctxt_alloc_init_args_t args;
    hal::pd::pd_func_args_t          pd_func_args = {0};


    pd_func_args.pd_cpupkt_ctxt_alloc_init = &args;
    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_ALLOC_INIT, &pd_func_args);

    return args.ctxt;
}

//------------------------------------------------------------------------------
// FTE instance constructor
//------------------------------------------------------------------------------
inst_t::inst_t(uint8_t fte_id) :
    id_(fte_id),
    arm_ctx_(fte_cpupkt_ctxt_alloc_init()),
    softq_(mpscq_t::alloc(MAX_SOFTQ_SLOTS)),
    ctx_(NULL),
    feature_state_(NULL),
    feature_state_size_(0),
    num_features_(0),
    iflow_(NULL),
    rflow_(NULL)
{
    hal_ret_t ret;
    hal::pd::pd_func_args_t          pd_func_args = {0};

    hal::pd::pd_cpupkt_register_rx_queue_args_t args;
    args.ctxt = arm_ctx_;
    args.type = types::WRING_TYPE_ARQRX;
    args.queue_id = fte_id;
    pd_func_args.pd_cpupkt_register_rx_queue = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_REG_RXQ, &pd_func_args);
    // ret = cpupkt_register_rx_queue(arm_ctx_, types::WRING_TYPE_ARQRX, fte_id);
    HAL_ASSERT(ret == HAL_RET_OK);

    args.type = types::WRING_TYPE_ARQTX;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_REG_RXQ, &pd_func_args);
    // ret = cpupkt_register_rx_queue(arm_ctx_, types::WRING_TYPE_ARQTX, fte_id);
    HAL_ASSERT(ret == HAL_RET_OK);

    hal::pd::pd_cpupkt_register_tx_queue_args_t tx_args;
    tx_args.ctxt = arm_ctx_;
    tx_args.type = types::WRING_TYPE_ASQ;
    tx_args.queue_id = fte_id;
    pd_func_args.pd_cpupkt_register_tx_queue = &tx_args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_REG_TXQ, &pd_func_args);
    // ret = cpupkt_register_tx_queue(arm_ctx_, types::WRING_TYPE_ASQ, fte_id);
    HAL_ASSERT(ret == HAL_RET_OK);
}

//------------------------------------------------------------------------------
// Allocate memory for fte::ctx_t
//------------------------------------------------------------------------------
void inst_t::ctx_mem_init()
{
    uint16_t num_features;
    size_t fstate_size = feature_state_size(&num_features);

    // Check if we need to realloc due to new feature registration
    if (ctx_ && (fstate_size != feature_state_size_ ||
                 num_features != num_features_)) {
        HAL_FREE(hal::HAL_MEM_ALLOC_FTE, ctx_);
        ctx_ = NULL;
    }

    if (!ctx_) {
        // Alloc memory for context, feature_state, iflows, rflows in one
        // contiguous area
        uint8_t *buff = (uint8_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE,
                                   sizeof(ctx_t) + fstate_size +
                                   2*ctx_t::MAX_STAGES*sizeof(flow_t));
        ctx_ = (ctx_t *)buff;
        buff += sizeof(ctx_t);

        iflow_ = (flow_t *)buff;
        buff += ctx_t::MAX_STAGES*sizeof(flow_t);

        rflow_ = (flow_t *)buff;
        buff += ctx_t::MAX_STAGES*sizeof(flow_t);

        feature_state_ = (feature_state_t*)buff;
        buff += fstate_size;
        feature_state_size_ = fstate_size;
        num_features_ = num_features;
    }
}

//------------------------------------------------------------------------------
// FTE main loop
//------------------------------------------------------------------------------
void inst_t::start()
{
    hal::hal_cfg_t *hal_cfg =
                (hal::hal_cfg_t *)hal::hal_get_current_thread()->data();
    HAL_ASSERT(hal_cfg);

    // Init logger for this instance if mode is set to true
    if (hal_cfg->shm_mode) {
        logger_ = ipc_logger::factory();
        HAL_ASSERT(logger_);
    }

    while(true) {
        if (hal_cfg->platform_mode == hal::HAL_PLATFORM_MODE_SIM) {
            usleep(1000000/3);
        } else if (hal_cfg->platform_mode == hal::HAL_PLATFORM_MODE_RTL) {
            usleep(1000000 * 3);
        }
        ctx_mem_init();
        process_arq();
        process_softq();
        process_tls_pendq();
    }
}

//------------------------------------------------------------------------------
// Send a packet on ASQ
//------------------------------------------------------------------------------
hal_ret_t
inst_t::asq_send(hal::pd::cpu_to_p4plus_header_t* cpu_header,
                 hal::pd::p4plus_to_p4_header_t* p4plus_header,
                 uint8_t* pkt, size_t pkt_len)
{
    HAL_TRACE_DEBUG("fte: sending pkt to id: {}", id_);
    hal::pd::pd_cpupkt_send_args_t args;
    hal::pd::pd_func_args_t pd_func_args = {0};
    args.ctxt = arm_ctx_;
    args.type = types::WRING_TYPE_ASQ;
    args.queue_id = id_;
    args.cpu_header = cpu_header;
    args.p4_header = p4plus_header;
    args.data = pkt;
    args.data_len = pkt_len;
    args.dest_lif = hal::SERVICE_LIF_CPU;
    args.qtype = CPU_ASQ_QTYPE;
    args.qid = id_;
    args.ring_number = CPU_SCHED_RING_ASQ;

    pd_func_args.pd_cpupkt_send = &args;
    return hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_SEND, &pd_func_args);
#if 0
    return hal::pd::cpupkt_send(arm_ctx_, types::WRING_TYPE_ASQ, id_,
                                cpu_header, p4plus_header, pkt, pkt_len,
                                hal::SERVICE_LIF_CPU, CPU_ASQ_QTYPE,
                                id_, CPU_SCHED_RING_ASQ);
#endif
}

//------------------------------------------------------------------------------
// Enqueue handler in FTE softq
//------------------------------------------------------------------------------
hal_ret_t
inst_t::softq_enqueue(softq_fn_t fn, void *data)
{
    sdk::lib::thread *curr_thread = hal::hal_get_current_thread();

    HAL_TRACE_DEBUG("fte: softq enqueue fte.{} fn={:p} data={:p}",
                    id_, (void*)fn, data);

    // try indefinatly until queued successfully
    while(softq_->enqueue((void *)fn, data) == false) {
        if (curr_thread->can_yield()) {
            pthread_yield();
        }
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Process an event from softq
//------------------------------------------------------------------------------
void inst_t::process_softq()
{
    void       *op;
    void       *data;

    if (softq_->dequeue(&op, &data)) {
        HAL_TRACE_DEBUG("fte: softq dequeue fn={:p} data={:p}", op, data);
        (*(softq_fn_t)op)(data);
    }
}

//------------------------------------------------------------------------------
// Process a pkt from arq
//------------------------------------------------------------------------------
void inst_t::process_arq()
{
    hal_ret_t ret;
    cpu_rxhdr_t *cpu_rxhdr;
    uint8_t *pkt;
    size_t pkt_len;

    // read the packet
    hal::pd::pd_cpupkt_poll_receive_args_t args;
    hal::pd::pd_func_args_t pd_func_args = {0};
    args.ctxt = arm_ctx_;
    args.flow_miss_hdr = &cpu_rxhdr;
    args.data = &pkt;
    args.data_len = &pkt_len;
    pd_func_args.pd_cpupkt_poll_receive = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_POLL_REC, &pd_func_args);
    // ret = hal::pd::cpupkt_poll_receive(arm_ctx_, &cpu_rxhdr, &pkt, &pkt_len);
    if (ret == HAL_RET_RETRY) {
        return;
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: arm rx failed, ret={}", ret);
        return;
    }

    // Process pkt with db open
    hal::hal_cfg_db_open(hal::CFG_OP_READ);

    do {
        // Init ctx_t
        ret = ctx_->init(cpu_rxhdr, pkt, pkt_len, iflow_, rflow_, feature_state_, num_features_);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: failed to init context, ret={}", ret);
            break;
        }

        // process the packet and update flow table
        auto app_ctx = hal::app_redir::app_redir_ctx(*ctx_, false);
        if (app_ctx) {
            app_ctx->set_arm_ctx(arm_ctx_);
        }
        ret = ctx_->process();
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: failied to process, ret={}", ret);
            break;
        }

        // write the packets
        ctx_->send_queued_pkts(arm_ctx_);
    } while(false);

    hal::hal_cfg_db_close();
}
//------------------------------------------------------------------------------
// Process a pkt from TLS asym pending request queue
//------------------------------------------------------------------------------
void inst_t::process_tls_pendq()
{
    hal::proxy::tls_poll_asym_pend_req_q();
}

} //   namespace fte
