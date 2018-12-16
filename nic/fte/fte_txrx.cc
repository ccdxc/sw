#include "nic/fte/fte.hpp"
#include "nic/fte/fte_impl.hpp"
#include "nic/fte/fte_softq.hpp"
#include "nic/fte/fte_flow.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/include/cpupkt_api.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/sdk/include/sdk/thread.hpp"

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
    void incr_feature_stats (uint16_t feature_id, hal_ret_t rc, bool set_rc);
    void incr_fte_error (hal_ret_t rc);
    fte_stats_t      get_stats(bool clear_on_read);
    fte_txrx_stats_t get_txrx_stats(bool clear_on_read);
    void update_rx_stats(cpu_rxhdr_t *rxhdr, size_t pkt_len);
    void update_tx_stats(size_t pkt_len);
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
    void ctx_mem_init();
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


//------------------------------------------------------------------------------
// FTE instance constructor
//------------------------------------------------------------------------------
inst_t::inst_t(uint8_t fte_id) :
    id_(fte_id),
    arm_ctx_(fte::impl::cpupkt_ctxt_alloc_init(fte_id)),
    softq_(mpscq_t::alloc(MAX_SOFTQ_SLOTS)),
    ctx_(NULL),
    feature_state_(NULL),
    feature_state_size_(0),
    num_features_(0),
    iflow_(NULL),
    rflow_(NULL)
{
    bzero((void *)&stats_, sizeof(fte_stats_t));
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
        if (hal::is_platform_type_hw()) {
            usleep(1000); //asicrw crash that was causing this should 
                           // been resolved by other changes to asicrw
        } else if (hal::is_platform_type_sim()) {
            usleep(1000000/30);
        } else if (hal::is_platform_type_rtl()) {
            usleep(1000000 * 3);
        } else if (hal::is_platform_type_haps()) {
            usleep(1000000/10);
        } else { /* PLATFORM_MOCK */
            usleep(1000);
        }
        process_arq();
        process_softq();
        fte::impl::process_pending_queues();
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
    HAL_TRACE_DEBUG("fte: sending pkt to id: {}, {}", id_, hex_str(pkt, pkt_len));
    return fte::impl::cpupkt_send(arm_ctx_, id_, cpu_header, p4plus_header, pkt, pkt_len);
}

//------------------------------------------------------------------------------
// Enqueue handler in FTE softq
//------------------------------------------------------------------------------
hal_ret_t
inst_t::softq_enqueue(softq_fn_t fn, void *data)
{
    sdk::lib::thread *curr_thread = hal::hal_get_current_thread();

    //HAL_TRACE_DEBUG("fte: softq enqueue fte.{} fn={:p} data={:p}",
    //                id_, (void*)fn, data);

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

        //HAL_TRACE_DEBUG("fte: softq dequeue fn={:p} data={:p} softq_req={}", op, data, stats_.softq_req);
        (*(softq_fn_t)op)(data);
    }
}

//----------------------------------------------------------------------------
// Increment feature drop counters
//----------------------------------------------------------------------------
void incr_inst_feature_stats(uint16_t feature_id, hal_ret_t rc, bool set_rc)
{
    if (fte_disabled_ || t_inst == NULL) {
        return;
    }

    t_inst->incr_feature_stats(feature_id, rc, set_rc);
}

//----------------------------------------------------------------------------
// Increment feature drop counters
//----------------------------------------------------------------------------
void inst_t::incr_feature_stats(uint16_t feature_id, hal_ret_t rc, bool set_rc)
{
    if (feature_id <= num_features_)
        stats_.feature_stats[feature_id].drop_pkts++;

    if (set_rc)
        stats_.feature_stats[feature_id].drop_reason[rc]++;
}

//----------------------------------------------------------------------------
// Increment fte error stats
//----------------------------------------------------------------------------
void incr_inst_fte_error(hal_ret_t rc)
{
    if (fte_disabled_ || t_inst == NULL) {
        return;
    }

    t_inst->incr_fte_error(rc);
}


//----------------------------------------------------------------------------
// Increment fte tx stats
//----------------------------------------------------------------------------
void incr_inst_fte_tx_stats(size_t pkt_len)
{
    if (fte_disabled_ || t_inst == NULL) {
        return;
    }

    t_inst->update_tx_stats(pkt_len);
}

//----------------------------------------------------------------------------
// Increment fte rx stats
//----------------------------------------------------------------------------
void incr_inst_fte_rx_stats(cpu_rxhdr_t *cpu_rxhdr, size_t pkt_len)
{
    if (fte_disabled_ || t_inst == NULL) {
        return;
    }

    t_inst->update_rx_stats(cpu_rxhdr, pkt_len);
}

//----------------------------------------------------------------------------
// Increment fte error counters
//----------------------------------------------------------------------------
void inst_t::incr_fte_error(hal_ret_t rc)
{
    stats_.fte_errors[rc]++;
}

//-----------------------------------------------------------------------------
 // Compute Connections per second
 //-----------------------------------------------------------------------------
 void inst_t::compute_cps(void)
 {
     sdk::timespec_t temp_ts;
     uint64_t         time_diff;

     // Get the current timestamp
     HAL_GET_SYSTEM_CLOCK(&t_cur_ts);
  
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
    } else if (lifq == NACL_REDIRECT_LIFQ) {
        stats_.redirect_pkts++;
    } else if (lifq == ALG_CFLOW_LIFQ) {
        stats_.cflow_pkts++;
    } else if (lifq == TCP_CLOSE_LIFQ) {
        stats_.tcp_close_pkts++;
    } else if (lifq == TLS_PROXY_LIFQ) {
        stats_.tls_proxy_pkts++;
    } else if (lifq == FTE_SPAN_LIFQ) {
        stats_.fte_span_pkts++;
    }
}

//-----------------------------------------------------------------------------
// Update Tx Counters
// ----------------------------------------------------------------------------
void inst_t::update_tx_stats(size_t pkt_len)
{
    stats_.queued_tx_pkts++;
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
    ret = fte::impl::cpupkt_poll_receive(arm_ctx_, &cpu_rxhdr, &pkt, &pkt_len);
    if (ret == HAL_RET_RETRY) {
        return;
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: arm rx failed, ret={}", ret);
        return;
    }

    // Process pkt with db open
    fte::impl::cfg_db_open();

    do {

        // Update Rx Counters
        update_rx_stats(cpu_rxhdr, pkt_len);

        // Init ctx_t
        ret = ctx_->init(cpu_rxhdr, pkt, pkt_len, iflow_, rflow_, feature_state_, num_features_);
        if (ret != HAL_RET_OK) {
            if (ret == HAL_RET_FTE_SPAN) {
                HAL_TRACE_DEBUG("fte: done processing span packet");
                continue;
            } else {
                HAL_TRACE_ERR("fte: failed to init context, ret={}", ret);
                break;
            }
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
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: failed to send pkt ret={}", ret);
        }
    } while(false);

    fte::impl::cfg_db_close();
}

//------------------------------------------------------------------------------
// API to fetch FTE stats on any given thread
//------------------------------------------------------------------------------
fte_stats_t inst_t::get_stats(bool clear_on_read)
{
    fte_stats_t stats = stats_;

    if (clear_on_read)
        bzero(&stats_, sizeof(fte_stats_t));

    return stats;
}

fte_txrx_stats_t inst_t::get_txrx_stats(bool clear_on_read)
{
    int      qindex = 0;
    uint8_t  queue_id = id_;
    //Get stats from the ctx
    hal::pd::cpupkt_ctxt_t *ctx = arm_ctx_;
    uint8_t  qinst = 0;  // For ARQ,ASCQ - qinst is always 0.
                         // For ASQ it is queueid
    //Fill common data
    fte_txrx_stats_t txrx_stats;

    txrx_stats.flow_miss_pkts = stats_.flow_miss_pkts;
    txrx_stats.redirect_pkts = stats_.redirect_pkts;
    txrx_stats.cflow_pkts    = stats_.cflow_pkts;
    txrx_stats.tcp_close_pkts = stats_.tcp_close_pkts;
    txrx_stats.tls_proxy_pkts = stats_.tls_proxy_pkts;

    //Get RX stats
    for (uint8_t i = 0; i < ctx->rx.num_queues; i++) {
        txrx_stats.qinfo[qindex].type  = ctx->rx.queue[i].type;
        txrx_stats.qinfo[qindex].inst.ctr = ctx->rx.queue[i].qinst_info[qinst]->ctr;
        txrx_stats.qinfo[qindex].inst.queue_id = ctx->rx.queue[i].qinst_info[qinst]->queue_id;
        txrx_stats.qinfo[qindex].inst.base_addr = ctx->rx.queue[i].qinst_info[qinst]->base_addr;
        txrx_stats.qinfo[qindex].inst.pc_index  = ctx->rx.queue[i].qinst_info[qinst]->pc_index;
        txrx_stats.qinfo[qindex].inst.pc_index_addr = ctx->rx.queue[i].qinst_info[qinst]->pc_index_addr;
        qindex++;
    }

    //GET Tx stats
    int index = types::WRING_TYPE_ASQ;  // pollmode driver uses wring type as index
    qinst = queue_id; //Same as fte id
    txrx_stats.qinfo[qindex].type = ctx->tx.queue[index].type;
    txrx_stats.qinfo[qindex].inst.ctr = ctx->tx.queue[index].qinst_info[qinst]->ctr;
    txrx_stats.qinfo[qindex].inst.queue_id = ctx->tx.queue[index].qinst_info[qinst]->queue_id;
    txrx_stats.qinfo[qindex].inst.base_addr = ctx->tx.queue[index].qinst_info[qinst]->base_addr;
    txrx_stats.qinfo[qindex].inst.pc_index  = ctx->tx.queue[index].qinst_info[qinst]->pc_index;
    txrx_stats.qinfo[qindex].inst.pc_index_addr = ctx->tx.queue[index].qinst_info[qinst]->pc_index_addr;

    // Fill common info
    //
    hal::pd::pd_cpupkt_get_global_args_t args;
    hal::pd::pd_func_args_t pd_func_args = {0};
    pd_func_args.pd_cpupkt_get_global = &args;

    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_GET_GLOBAL, &pd_func_args);
    txrx_stats.glinfo.gc_pindex = args.gc_pindex;
    txrx_stats.glinfo.cpu_tx_page_pindex = args.cpu_tx_page_pindex;
    txrx_stats.glinfo.cpu_tx_page_cindex = args.cpu_tx_page_cindex;
    txrx_stats.glinfo.cpu_tx_descr_pindex = args.cpu_tx_descr_pindex;
    txrx_stats.glinfo.cpu_tx_descr_cindex = args.cpu_tx_descr_cindex;
    return txrx_stats;

}

//------------------------------------------------------------------------------
// API to get txrxstats per thread
//------------------------------------------------------------------------------
fte_txrx_stats_t
fte_txrx_stats_get (uint8_t fte_id, bool clear_on_read)
{
    struct fn_ctx_t {
        fte_txrx_stats_t fte_stats;
        bool             clear_on_read;
    } fn_ctx;

    if (fte_disabled_)
        goto done;

    fn_ctx.clear_on_read = clear_on_read;
    fte_execute(fte_id, [](void *data) {
            fn_ctx_t *fn_ctx = (fn_ctx_t *) data;            
            fn_ctx->fte_stats = t_inst->get_txrx_stats(fn_ctx->clear_on_read);
        }, &fn_ctx);

done:
    return fn_ctx.fte_stats;
}


//------------------------------------------------------------------------------
// API to get stats per thread
//------------------------------------------------------------------------------

fte_stats_t
fte_stats_get (uint8_t fte_id, bool clear_on_read)
{
    struct fn_ctx_t {
        fte_stats_t fte_stats;
        bool        clear_on_read;
    } fn_ctx;

    if (fte_disabled_)
        goto done;

    fn_ctx.clear_on_read = clear_on_read;
    fte_execute(fte_id, [](void *data) {
            fn_ctx_t *fn_ctx = (fn_ctx_t *) data;
            fn_ctx->fte_stats = t_inst->get_stats(fn_ctx->clear_on_read);
        }, &fn_ctx);

done:
    return fn_ctx.fte_stats;
}

fte_stats_t& fte_stats_t::operator+=(const fte_stats_t& rhs) {
    cps += rhs.cps;
    flow_miss_pkts += rhs.flow_miss_pkts;
    redirect_pkts += rhs.redirect_pkts;
    cflow_pkts += rhs.cflow_pkts;
    tcp_close_pkts += rhs.tcp_close_pkts;
    softq_req += rhs.softq_req;
    queued_tx_pkts += rhs.queued_tx_pkts;
    for (uint8_t idx=0; idx<HAL_RET_ERR; idx++)
        fte_errors[idx] += rhs.fte_errors[idx];
    for (uint8_t idx=0; idx<get_num_features(); idx++) {
        feature_stats[idx].drop_pkts += \
                                  rhs.feature_stats[idx].drop_pkts;
        for (uint8_t rc=0; rc<HAL_RET_ERR; rc++)
            feature_stats[idx].drop_reason[rc] += \
                                            rhs.feature_stats[idx].drop_reason[rc];
    }

    return *this;
}

std::ostream& operator<<(std::ostream& os, const fte_stats_t& val)
{
    os << "{cps=" << val.cps;
    os << " ,flow_miss_pkts=" << val.flow_miss_pkts;
    os << " ,redirect_pkts=" << val.redirect_pkts;
    os << " ,cflow_pkts=" << val.cflow_pkts;
    os << " ,tcp_close_pkts=" << val.tcp_close_pkts;
    os << " ,tls_proxy_pkts=" << val.tls_proxy_pkts;
    os << " ,softq_req=" <<  val.softq_req;
    os << " ,queued_tx_pkts=" << val.queued_tx_pkts;
    os << " { FTE Errors: ";
    for (uint8_t idx=0; idx<HAL_RET_ERR; idx++) {
        if (val.fte_errors[idx]) {
            os << " ," << HAL_RET_ENTRIES_str((hal_ret_t)idx) << "=" << val.fte_errors[idx];
        }
    }
    os << " }, { Feature Drop Counters: ";
    for (uint8_t idx=0; idx<get_num_features(); idx++) {
        std::string      feature_name = fte::feature_id_to_name(idx);
        os <<  feature_name.substr(feature_name.find(":")+1) << " : ";
        os << " drop pkts=" << val.feature_stats[idx].drop_pkts;
        os << " { Feature Errors: ";
        for (uint8_t rc=0; rc<HAL_RET_ERR; rc++) {
            if (val.feature_stats[idx].drop_reason[rc]) {
                os << " ," << HAL_RET_ENTRIES_str((hal_ret_t)rc) << "=" << val.feature_stats[idx].drop_reason[rc];
            }
        }
        os << " } ";
    }
    return os << "} }";
}

} //   namespace fte
