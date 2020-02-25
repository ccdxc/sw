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
#include "lib/thread/thread.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/sdk/platform/capri/capri_hbm_rw.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "gen/proto/delphi.pb.h"
#include "gen/proto/ftestats/ftestats.delphi.hpp"
#include "nic/hal/pd/hal_pd.hpp"

#define FTE_EXPORT_STATS_SIZE     7
#define FTE_LIFQ_METRICS_OFFSET   16
#define FTE_MAX_SOFTQ_BATCH_SZ    128
namespace hal {
extern hal::session_stats_t  *g_session_stats;
extern hal_state *g_hal_state;

}
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
    void incr_fte_retransmit_packets(void);
    fte_stats_t      get_stats(bool clear_on_read);
    fte_txrx_stats_t get_txrx_stats(bool clear_on_read);
    void update_rx_stats(cpu_rxhdr_t *rxhdr, size_t pkt_len);
    void update_rx_stats_batch(uint16_t pktcount);
    void update_tx_stats(uint16_t pktcount);
    void set_bypass_fte(bool bypass_fte) { bypass_fte_ = bypass_fte; }
    void set_fte_max_sessions(uint64_t max_sessions) { max_sessions_ = max_sessions; }
    void incr_freed_tx_stats(void);
    void compute_cps();
    uint16_t softq_stats_get();

private:
    uint8_t                 id_;
    hal::pd::cpupkt_ctxt_t *arm_ctx_;
    void                   *tcp_ctx_;
    mpscq_t                *softq_;

    ctx_t                  *ctx_;
    feature_state_t        *feature_state_;
    size_t                  feature_state_size_;
    uint16_t                num_features_;
    flow_t                 *iflow_;
    flow_t                 *rflow_;
    ipc_logger             *logger_;
    fte_stats_t             stats_;
    bool                    bypass_fte_;
    timespec_t              t_old_ts_, t_cur_ts_;
    uint64_t                time_diff;
    uint64_t                max_sessions_; // sessions per inst;

    void process_arq();
    void process_arq_new();
    void process_softq();
    void ctx_mem_init();
    void compute_pps();
    void compute_pps_batch(uint16_t);
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
//----------------------------------------------------------------------------
thread_local uint64_t t_rx_pkts;
thread_local uint64_t t_rx_cxn;

//------------------------------------------------------------------------------
// FTE main pkt loop
// Creates FTE instance and starts it
//------------------------------------------------------------------------------
void
fte_start(void *ctxt)
{
    sdk::lib::thread   *curr_thread = (sdk::lib::thread *)ctxt;
    hal::hal_cfg_t     *hal_cfg = (hal::hal_cfg_t *)curr_thread->data();
    hal_ret_t          ret = HAL_RET_OK;
    uint8_t            fte_id;

    // do platform dependent clock delta computation initialization
    if (hal_cfg->features != hal::HAL_FEATURE_SET_GFT) {
        ret = hal::pd::hal_pd_clock_delta_comp_init(hal_cfg);
        SDK_ASSERT(ret == HAL_RET_OK);
        HAL_TRACE_DEBUG("Platform clock delta computation init done");
    }

    fte_id = curr_thread->thread_id() - hal::HAL_THREAD_ID_FTE_MIN;
    SDK_ASSERT(t_inst == NULL);
    SDK_ASSERT(fte_id < hal::MAX_FTE_THREADS);
    SDK_ASSERT(g_inst_list[fte_id] == NULL);

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
    if (fte_disabled_) {
        return HAL_RET_OK;
    }

    SDK_ASSERT_RETURN(t_inst, HAL_RET_INVALID_ARG);
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

    SDK_ASSERT_RETURN(t_inst, HAL_RET_INVALID_ARG);
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

    SDK_ASSERT_RETURN(t_inst, NULL);
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
        return (HAL_RET_OK);
    }

    SDK_ASSERT_RETURN(fte_id < hal::MAX_FTE_THREADS, HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN(fn, HAL_RET_INVALID_ARG);

    inst_t *inst = g_inst_list[fte_id];

    if (inst == NULL) {
        HAL_TRACE_ERR("fte: fte.{} is not initialized", fte_id);
        return HAL_RET_ERR;
    }

    return inst->softq_enqueue(fn, data);
}
typedef int softq_stats_t;
hal_ret_t 
fte_softq_stats_get(uint8_t fte_id, int &stat) {
    if (fte_disabled_) {
        return HAL_RET_OK;
    }
    if (g_inst_list[fte_id] != NULL) {
        stat = g_inst_list[fte_id]->softq_stats_get();
        return HAL_RET_OK;
    }
    return HAL_RET_ERR; 
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
    tcp_ctx_(NULL),
    softq_(mpscq_t::alloc(MAX_SOFTQ_SLOTS)),
    ctx_(NULL),
    feature_state_(NULL),
    feature_state_size_(0),
    num_features_(0),
    iflow_(NULL),
    rflow_(NULL),
    bypass_fte_(false),
    max_sessions_(0)
{
    if (hal::is_platform_type_sim()) {
        // SIM
        // Libmodel client doesnt have support to map hbm to shared memory today so
        // we cannot work with virtual address. Hence these stats will not be registered
        // with delphi until that is done
        stats_.fte_hbm_stats = (fte_hbm_stats_t *)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE, sizeof(fte_hbm_stats_t));
        SDK_ASSERT(stats_.fte_hbm_stats != NULL);
    } else {
        sdk::types::mem_addr_t vaddr;
        sdk::types::mem_addr_t start_addr = get_mem_addr(CAPRI_HBM_REG_PER_FTE_STATS);
        HAL_TRACE_VERBOSE("Start address: {:p}", (void *)start_addr);
        SDK_ASSERT(start_addr != INVALID_MEM_ADDRESS);

        bzero((void *)&stats_, sizeof(fte_stats_t));
        start_addr += fte_id << FTE_EXPORT_STATS_SIZE;

        // Register with Delphi with the physical address
        auto fte_cps_stats =
               delphi::objects::FteCPSMetrics::NewFteCPSMetrics(id_, start_addr);
        SDK_ASSERT(fte_cps_stats != NULL);

        auto fte_lifq_stats =
                 delphi::objects::FteLifQMetrics::NewFteLifQMetrics(id_, (start_addr + FTE_LIFQ_METRICS_OFFSET));
        SDK_ASSERT(fte_lifq_stats != NULL);

        sdk::lib::pal_ret_t pal_ret = sdk::lib::pal_physical_addr_to_virtual_addr(start_addr, &vaddr);
        HAL_TRACE_VERBOSE("Pal ret: {}", pal_ret);
        SDK_ASSERT(pal_ret == sdk::lib::PAL_RET_OK);
        stats_.fte_hbm_stats = (fte_hbm_stats_t *)vaddr;
        bzero(stats_.fte_hbm_stats, sizeof(fte_hbm_stats_t));
    }
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
    SDK_ASSERT(hal_cfg);

    HAL_TRACE_INFO("Starting FTE instance");
    HAL_TRACE_FLUSH();
    if (hal::is_platform_type_hw() && !getenv("DISABLE_FWLOG")) {
        logger_ = ipc_logger::factory();
        SDK_ASSERT(logger_);
    }

    /*
     * Get the bypass-fte flag from hal-config.
     */
    bypass_fte_ = hal_cfg->bypass_fte;
    max_sessions_ = hal_cfg->max_sessions;
    HAL_TRACE_DEBUG("Set max sessions:{}", max_sessions_);

    ctx_mem_init();

    // Init the tcp rings ctx
    tcp_ctx_ = fte::impl::init_tcp_rings_ctxt(id_, arm_ctx_);

    while (true) {
        process_arq_new();
        curr_thread->punch_heartbeat();
        process_softq();

#ifdef SIM
        fte::impl::process_pending_queues();
        ctx_->process_tcp_queues(tcp_ctx_);
#endif
        curr_thread->punch_heartbeat();

        if (hal::is_platform_type_hw()) {

            /*
             * Ideally, no sleep between packets on HW. However, currently
             * there is a starvation issue for mnic (and potentially other linux threads)
             * if FTE threads spin constantly, due to cpu affinities not setup properly.
             * Will be removing the sleep once those issues are addressed.
             * If 'bypass_fte_' is set, this is for PMD perf-mode, we dont want to sleep
             * between packets.
             */
            if (!bypass_fte_) usleep(10);
            continue;
        } else if (hal::is_platform_type_sim()) {
            usleep(1000000/30);
        } else if (hal::is_platform_type_rtl()) {
            usleep(1000000 * 3);
        } else if (hal::is_platform_type_haps()) {
            usleep(1000000/10);
        } else { /* PLATFORM_MOCK */
            usleep(1000);
        }
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
    HAL_TRACE_VERBOSE("fte: sending pkt to id: {}, {}", id_, hex_str(pkt, pkt_len));
    return fte::impl::cpupkt_send(arm_ctx_, id_, cpu_header, p4plus_header, pkt, pkt_len);
}

//------------------------------------------------------------------------------
// Enqueue handler in FTE softq
//------------------------------------------------------------------------------
hal_ret_t
inst_t::softq_enqueue(softq_fn_t fn, void *data)
{
    sdk::lib::thread *curr_thread = hal::hal_get_current_thread();

    // try indefinatly until queued successfully
    while(softq_->enqueue((void *)fn, data) == false) {
        if (curr_thread->can_yield()) {
            pthread_yield();
        }
    }
    return HAL_RET_OK;
}

uint16_t
inst_t::softq_stats_get()
{
    return softq_->get_queue_len();
}      

//------------------------------------------------------------------------------
// Process an event from softq
//------------------------------------------------------------------------------
void inst_t::process_softq()
{
    void       *op;
    void       *data;
    uint32_t    npkt=0;

    // Dequeue if we havent processed FTE_MAX_SOFTQ_BATCH_SZ requests yet
    while (npkt < FTE_MAX_SOFTQ_BATCH_SZ && softq_->dequeue(&op, &data)) {
        //Increment stats
        stats_.fte_hbm_stats->qstats.softq_req++;
        //compute_pps();

        (*(softq_fn_t)op)(data);
        npkt++;
    }
    if (npkt) {
        HAL_TRACE_VERBOSE("Done processing softq: {}", npkt);
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

    t_inst->update_tx_stats(1);
}

//----------------------------------------------------------------------------
// Increment fte tx stats
//----------------------------------------------------------------------------
void incr_inst_fte_tx_stats_batch (uint16_t pktcount)
{
    if (fte_disabled_ || t_inst == NULL) {
        return;
    }

    t_inst->update_tx_stats(pktcount);
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

//----------------------------------------------------------------------------
// Increment fte error counters
//----------------------------------------------------------------------------
void inst_t::incr_fte_retransmit_packets(void)
{
    stats_.fte_hbm_stats->qstats.flow_retransmit_pkts++;
}

//----------------------------------------------------------------------------
// Increment fte existing session counters
//----------------------------------------------------------------------------
void incr_fte_retransmit_packets(void)
{
    if (fte_disabled_ || t_inst == NULL) {
        return;
    }

    t_inst->incr_fte_retransmit_packets();
}

//----------------------------------------------------------------------------
// Increment fte tx stats
//----------------------------------------------------------------------------
void incr_inst_freed_tx_stats(void)
{
    if (fte_disabled_ || t_inst == NULL) {
        return;
    }

    t_inst->incr_freed_tx_stats();
}

//----------------------------------------------------------------------------
// Increment Freed CPU Tx Resources
//----------------------------------------------------------------------------
void inst_t::incr_freed_tx_stats(void)
{
    stats_.fte_hbm_stats->qstats.freed_tx_pkts++;
}

//----------------------------------------------------------------------------
// Compute CPS on the given FTE instance
//----------------------------------------------------------------------------
void fte_inst_compute_cps(void)
{
    if (fte_disabled_ || t_inst == NULL) {
        return;
    }

    t_inst->compute_cps();
}

//-----------------------------------------------------------------------------
// Compute Connections per second
//-----------------------------------------------------------------------------
void inst_t::compute_cps(void)
{
    if (time_diff < TIME_NSECS_PER_SEC) {
        t_rx_cxn++;
    }
}

//-----------------------------------------------------------------------------
// Compute packets per second
//-----------------------------------------------------------------------------
void inst_t::compute_pps(void)
{
     sdk::timespec_t temp_ts;

     // Get the current timestamp
     HAL_GET_SYSTEM_CLOCK(&t_cur_ts_);

     temp_ts = t_cur_ts_;
     sdk::timestamp_subtract(&temp_ts, &t_old_ts_);
     sdk::timestamp_to_nsecs(&temp_ts, &time_diff);

     if (time_diff > TIME_NSECS_PER_SEC) {
         stats_.fte_hbm_stats->cpsstats.pps = t_rx_pkts;
         stats_.fte_hbm_stats->cpsstats.cps = t_rx_cxn;
         t_old_ts_ = t_cur_ts_;
         t_rx_pkts = 1;
         t_rx_cxn = 1;
     } else if (time_diff == TIME_NSECS_PER_SEC) {
         stats_.fte_hbm_stats->cpsstats.pps = ++t_rx_pkts;
         stats_.fte_hbm_stats->cpsstats.cps = ++t_rx_cxn;
         t_old_ts_ = t_cur_ts_;
     } else {
         t_rx_pkts++;
     }

     // Record the Max. PPS we've done
     if (stats_.fte_hbm_stats->cpsstats.pps > stats_.fte_hbm_stats->cpsstats.pps_hwm)
         stats_.fte_hbm_stats->cpsstats.pps_hwm = stats_.fte_hbm_stats->cpsstats.pps;
 
     // Record the Max. CPS we've done
     if (stats_.fte_hbm_stats->cpsstats.cps > stats_.fte_hbm_stats->cpsstats.cps_hwm)
        stats_.fte_hbm_stats->cpsstats.cps_hwm = stats_.fte_hbm_stats->cpsstats.cps;
}

//-----------------------------------------------------------------------------
// Compute packets per second
//-----------------------------------------------------------------------------
void inst_t::compute_pps_batch(uint16_t pktcount)
{
     sdk::timespec_t temp_ts;
     uint64_t         time_diff;

     // Get the current timestamp
     HAL_GET_SYSTEM_CLOCK(&t_cur_ts_);

     temp_ts = t_cur_ts_;
     sdk::timestamp_subtract(&temp_ts, &t_old_ts_);
     sdk::timestamp_to_nsecs(&temp_ts, &time_diff);

     if (time_diff > TIME_NSECS_PER_SEC) {
         stats_.fte_hbm_stats->cpsstats.pps = t_rx_pkts;
         t_old_ts_ = t_cur_ts_;
         t_rx_pkts = pktcount;
     } else if (time_diff == TIME_NSECS_PER_SEC) {
         t_rx_pkts += pktcount;
         stats_.fte_hbm_stats->cpsstats.pps = t_rx_pkts;
         t_old_ts_ = t_cur_ts_;
     } else {
         t_rx_pkts += pktcount;
     }

     // Record the Max. PPS we've done
     if (stats_.fte_hbm_stats->cpsstats.pps > stats_.fte_hbm_stats->cpsstats.pps_hwm)
         stats_.fte_hbm_stats->cpsstats.pps_hwm = stats_.fte_hbm_stats->cpsstats.pps;

 }

//-----------------------------------------------------------------------------
// Update Rx Counters based on the queue
// ----------------------------------------------------------------------------
void inst_t::update_rx_stats(cpu_rxhdr_t *cpu_rxhdr, size_t pkt_len)
{
    /*
     * If no cpu header, then this is a dummy flow-miss packet for PPS testing,
     * we'll still compute CPS.
     */
    if (bypass_fte_ && !cpu_rxhdr) {
        compute_pps();
        stats_.fte_hbm_stats->qstats.flow_miss_pkts++;
	return;
    }

    lifqid_t lifq = {cpu_rxhdr->lif, cpu_rxhdr->qtype, cpu_rxhdr->qid};

    compute_pps();

    if (lifq == FLOW_MISS_LIFQ) {
        stats_.fte_hbm_stats->qstats.flow_miss_pkts++;
    } else if (lifq == NACL_REDIRECT_LIFQ) {
        stats_.fte_hbm_stats->qstats.redirect_pkts++;
    } else if (lifq == ALG_CFLOW_LIFQ) {
        stats_.fte_hbm_stats->qstats.cflow_pkts++;
    } else if (lifq == TCP_CLOSE_LIFQ) {
        stats_.fte_hbm_stats->qstats.tcp_close_pkts++;
    } else if (lifq == TLS_PROXY_LIFQ) {
        stats_.fte_hbm_stats->qstats.tls_proxy_pkts++;
    } else if (lifq == FTE_SPAN_LIFQ) {
        stats_.fte_hbm_stats->qstats.fte_span_pkts++;
    }
}

//-----------------------------------------------------------------------------
// Update Rx Counters based on the queue
// ----------------------------------------------------------------------------
void inst_t::update_rx_stats_batch(uint16_t pktcount)
{
    /*
     * If no cpu header, then this is a dummy flow-miss packet for PPS testing,
     * we'll still compute CPS.
     */
    if (!bypass_fte_) return;

    compute_pps_batch(pktcount);
    stats_.fte_hbm_stats->qstats.flow_miss_pkts += pktcount;
    return;
}


//-----------------------------------------------------------------------------
// Update Tx Counters
// ----------------------------------------------------------------------------
void inst_t::update_tx_stats(uint16_t pktcount)
{
    stats_.fte_hbm_stats->qstats.queued_tx_pkts += pktcount;
}

void free_flow_miss_pkt(uint8_t * pkt)
{
    hal::free_to_slab(hal::HAL_SLAB_CPU_PKT, (pkt-sizeof(cpu_rxhdr_t)));
}

//------------------------------------------------------------------------------
// Process a pkt from arq
//------------------------------------------------------------------------------
void inst_t::process_arq()
{
    hal_ret_t ret;
    cpu_rxhdr_t *cpu_rxhdr;
    uint8_t *pkt = NULL;
    size_t pkt_len;
    bool   copied_pkt = true;
    auto app_ctx = hal::app_redir::app_redir_ctx(*ctx_, false);

    // read the packet
    ret = fte::impl::cpupkt_poll_receive(arm_ctx_, &cpu_rxhdr, &pkt, &pkt_len, &copied_pkt);
    if (ret == HAL_RET_RETRY) {
        return;
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: arm rx failed, ret={}", ret);
        return;
    }

    /*
     * In 'bypass_fte' mode, we just want to go thru the CPU PMD Rx and Tx paths. This
     * mode is used for PMD PPS measurements/testing. So we'll just enqueue the packet
     * to tx-q and send it out.
     */
    if (bypass_fte_) {

        HAL_TRACE_VERBOSE("CPU-PMD: Bypassing FTE processing!! pkt={:p}\n", pkt);

        update_rx_stats(cpu_rxhdr, pkt_len);
        if (pkt) {

            ctx_->set_pkt(cpu_rxhdr, pkt, pkt_len);
            hal::pd::cpu_to_p4plus_header_t cpu_header = {0};

            // Update Rx Counters
            //update_rx_stats(cpu_rxhdr, pkt_len);

            /*
             * If the 'copied_pkt' is not set, then this is not a packet buffer
             * that we've allocated from slab, so no need to free it.
             */
            ctx_->queue_txpkt(pkt, pkt_len, &cpu_header, NULL, HAL_LIF_CPU,
                              CPU_ASQ_QTYPE, CPU_ASQ_QID, CPU_SCHED_RING_ASQ,
                              types::WRING_TYPE_ASQ, copied_pkt ? free_flow_miss_pkt : NULL);

            // write the packet
            ret = ctx_->send_queued_pkts(arm_ctx_);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("fte: failed to send pkt ret={}", ret);
            }

	    // The 'send_queued_pkts()' already updates tx stats.
            //update_tx_stats(pkt_len);
        }
        return;
    }

    // Process pkt with db open
    fte::impl::cfg_db_open();

    do {

        // Update Rx Counters
        update_rx_stats(cpu_rxhdr, pkt_len);

        // Init ctx_t
        ret = ctx_->init(cpu_rxhdr, pkt, pkt_len, copied_pkt, iflow_, rflow_, feature_state_, num_features_);
        if (ret != HAL_RET_OK) {
            if (ret == HAL_RET_FTE_SPAN) {
                HAL_TRACE_VERBOSE("fte: done processing span packet");
                continue;
            } else if (ret == HAL_RET_RETRANSMISSION) {
                HAL_TRACE_VERBOSE("fte: retransmission packet");
                goto send;
            } else {
                HAL_TRACE_ERR("fte: failed to init context, ret={}", ret);
                break;
            }
        }

        // process the packet and update flow table
        if (app_ctx) {
            app_ctx->set_arm_ctx(arm_ctx_);
        }
        ret = ctx_->process();
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: failed to process, ret={}", ret);

            /*
             * We'll set the drop in this error case, so the cpupkt resources
             * can be reclaimed in 'send_queued_pkts' below.
             */
	        ctx_->set_drop();
        }

send:
        // write the packets
        ret = ctx_->send_queued_pkts(arm_ctx_);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: failed to send pkt ret={}", ret);
        }
    } while(false);

    fte::impl::cfg_db_close();
}

thread_local hal::pd::cpupkt_pkt_batch_t cpupkt_batch;

//------------------------------------------------------------------------------
// Process pkt(s) from arq
//------------------------------------------------------------------------------
void inst_t::process_arq_new ()
{
    hal_ret_t                 ret;
    uint16_t                  npkt;
    cpu_rxhdr_t               *cpu_rxhdr;
    uint8_t                   *pkt = NULL;
    size_t                    pkt_len;
    bool                      copied_pkt, drop_pkt;

    // reset the batch packet count
    cpupkt_batch.pktcount = 0;

    // read the packet
    ret = fte::impl::cpupkt_poll_receive_new(arm_ctx_, &cpupkt_batch);
    if (ret == HAL_RET_RETRY) {
        return;
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: arm rx failed, ret={}", ret);
        return;
    }

    /*
     * In 'bypass_fte' mode, we just want to go thru the CPU PMD Rx and Tx paths. This
     * mode is used for PMD PPS measurements/testing. So we'll just enqueue the packet
     * to tx-q and send it out.
     */
    if (bypass_fte_) {

        update_rx_stats_batch(cpupkt_batch.pktcount);

        for (npkt = 0; npkt < cpupkt_batch.pktcount; npkt++) {
            pkt = cpupkt_batch.pkts[npkt].pkt;
            pkt_len = cpupkt_batch.pkts[npkt].pkt_len;
            cpu_rxhdr = cpupkt_batch.pkts[npkt].cpu_rxhdr;
            copied_pkt = cpupkt_batch.pkts[npkt].copied_pkt;

            HAL_TRACE_VERBOSE("CPU-PMD: Bypassing FTE processing!! pkt={:p}\n", pkt);

            if (pkt) {

                ctx_->set_pkt(cpu_rxhdr, pkt, pkt_len);
                hal::pd::cpu_to_p4plus_header_t cpu_header = {0};

                // Update Rx Counters
                //update_rx_stats(cpu_rxhdr, pkt_len);

                ctx_->queue_txpkt(pkt, pkt_len, &cpu_header, NULL, HAL_LIF_CPU,
                                  CPU_ASQ_QTYPE, CPU_ASQ_QID, CPU_SCHED_RING_ASQ,
                                  types::WRING_TYPE_ASQ, copied_pkt ? free_flow_miss_pkt : NULL);
            }
        }

        // write the packet
        ret = ctx_->send_queued_pkts_new(arm_ctx_);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: failed to send pkt ret={}", ret);
        }

        // The 'send_queued_pkts()' already updates tx stats.
        //update_tx_stats(pkt_len);

        return;
    }

    if (!cpupkt_batch.pktcount) return;

    HAL_TRACE_VERBOSE("Received {} packets", cpupkt_batch.pktcount);

    for (npkt = 0; npkt < cpupkt_batch.pktcount; npkt++) {

        // Process pkt with db open
        fte::impl::cfg_db_open();

        pkt = cpupkt_batch.pkts[npkt].pkt;
        pkt_len = cpupkt_batch.pkts[npkt].pkt_len;
        cpu_rxhdr = cpupkt_batch.pkts[npkt].cpu_rxhdr;
        copied_pkt = cpupkt_batch.pkts[npkt].copied_pkt;
        drop_pkt = false;

        HAL_TRACE_VERBOSE("npkt {} pkt_len {}, pkt {:p}", npkt, pkt_len, pkt);

        do {

            // Update Rx Counters
            update_rx_stats(cpu_rxhdr, pkt_len);

            // Init ctx_t
            ret = ctx_->init(cpu_rxhdr, pkt, pkt_len, copied_pkt, iflow_, rflow_, feature_state_, num_features_);
            if (ret != HAL_RET_OK) {
                if (ret == HAL_RET_FTE_SPAN) {
                    HAL_TRACE_VERBOSE("fte: done processing span packet");
                } else {
                    HAL_TRACE_ERR("fte: failed to init context, ret={}", ret);
                }

                /*
                 * Set drop bit so any packet-resources can be freed by the CPU-PMD.
                 */
                drop_pkt = true;
                ctx_->set_drop();
            }

            if (hal::g_hal_state->is_base_net()) {
                HAL_TRACE_ERR("FTE should not receive any pkts in Base Net.");
                drop_pkt = true;
                ctx_->set_drop();
                continue;
            }

            if ((drop_pkt == false) && hal::g_session_stats && 
                hal::g_session_stats[id_].total_active_sessions >= max_sessions_) {
                drop_pkt = true;
                stats_.fte_hbm_stats->qstats.max_session_drop_pkts++;
                ctx_->set_drop();
            }

            if (!drop_pkt) {

                // process the packet and update flow table
                auto app_ctx = hal::app_redir::app_redir_ctx(*ctx_, false);
                if (app_ctx) {
                    app_ctx->set_arm_ctx(arm_ctx_);
                }
                ret = ctx_->process();
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("fte: failied to process, ret={}", ret);

                    /*
                     * We'll set the drop in this error case, so the cpupkt resources
                     * can be reclaimed in 'send_queued_pkts' below.
                     */
                    ctx_->set_drop();
                }
            }

            // write the packets
            ret = ctx_->send_queued_pkts(arm_ctx_);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("fte: failed to send pkt ret={}", ret);
            }
        } while(false);

        fte::impl::cfg_db_close();
    }
    HAL_TRACE_VERBOSE("Done processing {} packets", cpupkt_batch.pktcount);

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

    txrx_stats.flow_miss_pkts = stats_.fte_hbm_stats->qstats.flow_miss_pkts;
    txrx_stats.redirect_pkts = stats_.fte_hbm_stats->qstats.redirect_pkts;
    txrx_stats.cflow_pkts    = stats_.fte_hbm_stats->qstats.cflow_pkts;
    txrx_stats.tcp_close_pkts = stats_.fte_hbm_stats->qstats.tcp_close_pkts;
    txrx_stats.tls_proxy_pkts = stats_.fte_hbm_stats->qstats.tls_proxy_pkts;

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
    txrx_stats.glinfo.cpu_tx_page_full_err = args.cpu_tx_page_full_err;
    txrx_stats.glinfo.cpu_tx_descr_pindex = args.cpu_tx_descr_pindex;
    txrx_stats.glinfo.cpu_tx_descr_cindex = args.cpu_tx_descr_cindex;
    txrx_stats.glinfo.cpu_tx_descr_full_err = args.cpu_tx_descr_full_err;
    txrx_stats.glinfo.cpu_rx_dpr_cindex = args.cpu_rx_dpr_cindex;
    txrx_stats.glinfo.cpu_rx_dpr_sem_cindex = args.cpu_rx_dpr_sem_cindex;
    txrx_stats.glinfo.cpu_rx_dpr_descr_free_err = args.cpu_rx_dpr_descr_free_err;
    txrx_stats.glinfo.cpu_rx_dpr_sem_free_err = args.cpu_rx_dpr_sem_free_err;
    txrx_stats.glinfo.cpu_rx_dpr_descr_invalid_free_err = args.cpu_rx_dpr_descr_invalid_free_err;
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
// API to set bypass FTE for upgrade
//------------------------------------------------------------------------------
void
set_bypass_fte (uint8_t fte_id, bool bypass_fte)
{
    struct fn_ctx_t {
        bool  bypass_fte;
    } fn_ctx;

    if (fte_disabled_)
        goto done;
    fn_ctx.bypass_fte = bypass_fte;
    fte_execute(fte_id, [](void *data) {
            fn_ctx_t *fn_ctx = (fn_ctx_t *) data;
            t_inst->set_bypass_fte(fn_ctx->bypass_fte);
        }, &fn_ctx);

done:
    return;
}


void
set_fte_max_sessions (uint8_t fte_id, uint64_t max_sessions)
{
    struct fn_ctx_t {
        uint64_t max_sessions;
    } fn_ctx;

    if (fte_disabled_)
        goto done;
    fn_ctx.max_sessions = max_sessions;
    HAL_TRACE_VERBOSE("max sessions {}", max_sessions);

    fte_execute(fte_id, [](void *data) {
            fn_ctx_t *fn_ctx = (fn_ctx_t *) data;
            t_inst->set_fte_max_sessions(fn_ctx->max_sessions);
        }, &fn_ctx);

done:
    return;
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

std::ostream& operator<<(std::ostream& os, const fte_stats_t& val)
{
    os << "{cps=" << val.fte_hbm_stats->cpsstats.cps;
    os << " ,flow_miss_pkts=" << val.fte_hbm_stats->qstats.flow_miss_pkts;
    os << " ,redirect_pkts=" << val.fte_hbm_stats->qstats.redirect_pkts;
    os << " ,cflow_pkts=" << val.fte_hbm_stats->qstats.cflow_pkts;
    os << " ,tcp_close_pkts=" << val.fte_hbm_stats->qstats.tcp_close_pkts;
    os << " ,tls_proxy_pkts=" << val.fte_hbm_stats->qstats.tls_proxy_pkts;
    os << " ,softq_req=" <<  val.fte_hbm_stats->qstats.softq_req;
    os << " ,queued_tx_pkts=" << val.fte_hbm_stats->qstats.queued_tx_pkts;
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
