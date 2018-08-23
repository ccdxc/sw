#include "nic/fte/fte.hpp"
#include "nic/fte/fte_softq.hpp"
#include "nic/fte/fte_flow.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/include/cpupkt_api.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "sdk/timestamp.hpp"
#include "sdk/thread.hpp"

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
    void start(sdk::lib::thread *curr_thread);
    hal_ret_t asq_send(hal::pd::cpu_to_p4plus_header_t* cpu_header,
                       hal::pd::p4plus_to_p4_header_t* p4plus_header,
                       uint8_t* pkt, size_t pkt_len);
    hal_ret_t softq_enqueue(softq_fn_t fn, void *data);
    uint8_t get_id() const {return id_;};
    ipc_logger *get_ipc_logger() const { return logger_; }
    void incr_feature_drop_stats(uint16_t feature_id);
    fte_stats_t get_stats() { return stats_; }

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
    fte_stats_t             stats_;

    void process_arq();
    void process_softq();
    void process_tls_pendq();
    void ctx_mem_init();
    void update_rx_stats(cpu_rxhdr_t *rxhdr, size_t pkt_len);
    void update_tx_stats(size_t pkt_len);
    void compute_cps();
};

//------------------------------------------------------------------------------
// Per thread FTE instances
//------------------------------------------------------------------------------
static inst_t *g_inst_list[hal::MAX_FTE_THREADS];

//------------------------------------------------------------------------------
// FTE instance of current thread
//------------------------------------------------------------------------------
thread_local inst_t *t_inst;

//-----------------------------------------------------------------------------
// FTE thread local variables
// ----------------------------------------------------------------------------
thread_local timespec_t t_old_ts;
thread_local timespec_t t_cur_ts;
thread_local uint64_t t_rx_pkts;

//------------------------------------------------------------------------------
// FTE main pkt loop
// Creates FTE instance and starts it
//------------------------------------------------------------------------------
void
fte_start(void *ctxt)
{
    sdk::lib::thread   *curr_thread = (sdk::lib::thread *)ctxt;
    uint8_t            fte_id;

    fte_id = curr_thread->thread_id() - hal::HAL_THREAD_ID_FTE_MIN;
    HAL_ASSERT(t_inst == NULL);
    HAL_ASSERT(fte_id < hal::MAX_FTE_THREADS);
    HAL_ASSERT(g_inst_list[fte_id] == NULL);

    t_inst = g_inst_list[fte_id] = new inst_t(fte_id);
    t_inst->start(curr_thread);
    HAL_TRACE_DEBUG("Started FTE thread: {}", fte_id);
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
    HAL_ASSERT_RETURN(t_inst, HAL_RET_INVALID_ARG);
    return t_inst->asq_send(cpu_header, p4plus_header, pkt, pkt_len);
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

    HAL_ASSERT_RETURN(t_inst, HAL_RET_INVALID_ARG);
    return t_inst->get_id();
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

    HAL_ASSERT_RETURN(t_inst, NULL);
    return t_inst->get_ipc_logger();
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

    HAL_ASSERT_RETURN(t_inst == NULL, HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(fte_id < hal::MAX_FTE_THREADS, HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(fn, HAL_RET_INVALID_ARG);

    inst_t *inst = g_inst_list[fte_id];

    if (inst == NULL) {
        HAL_TRACE_ERR("fte: fte.{} is not initialized", fte_id);
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
    hal_ret_t                 ret;
    hal::pd::pd_func_args_t   pd_func_args = {0};
    uint16_t                  num_features;

    hal::pd::pd_cpupkt_register_rx_queue_args_t args;
    args.ctxt = arm_ctx_;
    args.type = types::WRING_TYPE_ARQRX;
    args.queue_id = fte_id;
    pd_func_args.pd_cpupkt_register_rx_queue = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_REG_RXQ, &pd_func_args);
    // ret = cpupkt_register_rx_queue(arm_ctx_, types::WRING_TYPE_ARQRX, fte_id);
    HAL_ASSERT(ret == HAL_RET_OK);

    args.type = types::WRING_TYPE_ASCQ;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_REG_RXQ, &pd_func_args);
    HAL_ASSERT(ret == HAL_RET_OK);

    hal::pd::pd_cpupkt_register_tx_queue_args_t tx_args;
    tx_args.ctxt = arm_ctx_;
    tx_args.type = types::WRING_TYPE_ASQ;
    tx_args.queue_id = fte_id;
    pd_func_args.pd_cpupkt_register_tx_queue = &tx_args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_REG_TXQ, &pd_func_args);
    // ret = cpupkt_register_tx_queue(arm_ctx_, types::WRING_TYPE_ASQ, fte_id);
    HAL_ASSERT(ret == HAL_RET_OK);

    bzero((void *)&stats_, sizeof(fte_stats_t));
    feature_state_size_ = feature_state_size(&num_features);
    stats_.feature_drop_pkts = (uint64_t *)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE,
                                                          feature_state_size_);
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
void inst_t::start(sdk::lib::thread *curr_thread)
{
    hal::hal_cfg_t *hal_cfg = (hal::hal_cfg_t *)curr_thread->data();
    HAL_ASSERT(hal_cfg);

    HAL_TRACE_DEBUG("Starting FTE instance: {}", hal_cfg->shm_mode);
    if (hal_cfg->shm_mode) {
        logger_ = ipc_logger::factory();
        HAL_ASSERT(logger_);
    }

    ctx_mem_init();
    while (true) {
        if (hal_cfg->platform_mode == hal::HAL_PLATFORM_MODE_SIM) {
            usleep(1000000/30);
        } else if (hal_cfg->platform_mode == hal::HAL_PLATFORM_MODE_RTL ||
                hal_cfg->platform_mode == hal::HAL_PLATFORM_MODE_HAPS) {
            usleep(1000000 * 3);
        }
        process_arq();
        process_softq();
        process_tls_pendq();
        curr_thread->punch_heartbeat();
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
        //Increment stats
        stats_.softq_req++;
        compute_cps();

        HAL_TRACE_DEBUG("fte: softq dequeue fn={:p} data={:p} softq_req={}", op, data, stats_.softq_req);
        (*(softq_fn_t)op)(data);
    }
}

//----------------------------------------------------------------------------
// Increment feature drop counters
//----------------------------------------------------------------------------
void incr_inst_feature_drop(uint16_t feature_id)
{
    if (fte_disabled_) {
        return;
    }

    HAL_ASSERT(t_inst != NULL);
    t_inst->incr_feature_drop_stats(feature_id);
}

//----------------------------------------------------------------------------
// Increment feature drop counters
//----------------------------------------------------------------------------
void inst_t::incr_feature_drop_stats(uint16_t feature_id)
{
    if (feature_id <= feature_state_size_)
        stats_.feature_drop_pkts[feature_id]++;
}

//-----------------------------------------------------------------------------
// Compute Connections per second
//-----------------------------------------------------------------------------
void inst_t::compute_cps(void)
{
    sdk::timespec_t temp_ts;
    uint64_t         time_diff;

    // Get the current timestamp
    clock_gettime(CLOCK_MONOTONIC, &t_cur_ts);

    temp_ts = t_cur_ts;
    sdk::timestamp_subtract(&temp_ts, &t_old_ts);
    sdk::timestamp_to_nsecs(&temp_ts, &time_diff);

    if (time_diff > TIME_NSECS_PER_SEC) {
        stats_.cps = t_rx_pkts;
        t_old_ts = t_cur_ts;
        t_rx_pkts = 1;
    } else if (time_diff == TIME_NSECS_PER_SEC) {
        stats_.cps = ++t_rx_pkts;
        t_old_ts = t_cur_ts;
    } else {
        t_rx_pkts++;
    }

}

//-----------------------------------------------------------------------------
// Update Rx Counters based on the queue
// ----------------------------------------------------------------------------
void inst_t::update_rx_stats(cpu_rxhdr_t *cpu_rxhdr, size_t pkt_len)
{
    lifqid_t lifq = {cpu_rxhdr->lif, cpu_rxhdr->qtype, cpu_rxhdr->qid};

    compute_cps();

    if (lifq == FLOW_MISS_LIFQ) {
        stats_.flow_miss_pkts++;
        stats_.flow_miss_bytes += pkt_len;
    } else if (lifq == NACL_REDIRECT_LIFQ) {
        stats_.redirect_pkts++;
        stats_.redirect_bytes += pkt_len;
    } else if (lifq == ALG_CFLOW_LIFQ) {
        stats_.cflow_pkts++;
        stats_.cflow_bytes += pkt_len;
    } else if (lifq == TCP_CLOSE_LIFQ) {
        stats_.tcp_close_pkts++;
        stats_.tcp_close_bytes += pkt_len;
    } else if (lifq == TLS_PROXY_LIFQ) {
        stats_.tls_proxy_pkts++;
        stats_.tls_proxy_bytes += pkt_len;
    }
}

//-----------------------------------------------------------------------------
// Update Tx Counters
// ----------------------------------------------------------------------------
void inst_t::update_tx_stats(size_t pkt_len)
{
    stats_.queued_tx_pkts++;
    stats_.queued_tx_bytes += pkt_len;
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

        // Update Rx Counters
        update_rx_stats(cpu_rxhdr, pkt_len);

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
        ret = ctx_->send_queued_pkts(arm_ctx_);
        if (ret == HAL_RET_OK) {
            update_tx_stats(pkt_len);
        }
    } while(false);

    hal::hal_cfg_db_close();
}
//------------------------------------------------------------------------------
// Process a pkt from TLS asym pending request queue
//------------------------------------------------------------------------------
void inst_t::process_tls_pendq()
{
    // TBD: Move the poll here to this can be
    // accounted towards the CPS calculations
    hal::proxy::tls_poll_asym_pend_req_q();
}

fte_stats_t
fte_get_stats (uint8_t fte_id)
{
    struct fn_ctx_t {
        fte_stats_t fte_stats;
    } fn_ctx;

    fte_execute(fte_id, [](void *data) {
            fn_ctx_t *fn_ctx = (fn_ctx_t *) data;
            fn_ctx->fte_stats = t_inst->get_stats();
        }, &fn_ctx);

    return fn_ctx.fte_stats;
}

} //   namespace fte
