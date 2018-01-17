#include "nic/fte/fte.hpp"
#include "nic/fte/fte_softq.hpp"
#include "nic/fte/fte_flow.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/p4/nw/include/defines.h"
#include "nic/include/cpupkt_api.hpp"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"

namespace fte {

static const uint16_t MAX_SOFTQ_SLOTS(1024);
static const uint8_t  MAX_FTE_THREADS =
    hal::HAL_THREAD_ID_FTE_MAX - hal::HAL_THREAD_ID_FTE_MIN + 1;

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

    void process_arq();
    void process_softq();
    void ctx_mem_init();
};

//------------------------------------------------------------------------------
// Per thread FTE instances
//------------------------------------------------------------------------------
static inst_t *g_inst_list[MAX_FTE_THREADS];

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
    HAL_ASSERT(fte_id < MAX_FTE_THREADS);
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
    HAL_ASSERT_RETURN(g_inst, HAL_RET_INVALID_ARG);
    return g_inst->get_id();
}


//------------------------------------------------------------------------------
// Asynchronouly executes the fn in the specified fte thread,
// If the softq is full, it blocks until a slot is empty.
// Should be called from non FTE thread
//-----------------------------------------------------------------------------
hal_ret_t
fte_softq_enqueue(uint8_t fte_id, softq_fn_t fn, void *data)
{
    HAL_ASSERT_RETURN(g_inst == NULL, HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(fte_id < MAX_FTE_THREADS, HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(fn, HAL_RET_INVALID_ARG);

    inst_t *inst = g_inst_list[fte_id];

    if (inst == NULL) {
        HAL_TRACE_ERR("fte: fte.{} is not initializd", fte_id);
        return HAL_RET_ERR;
    }

    return inst->softq_enqueue(fn, data);
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
    arm_ctx_(hal::pd::cpupkt_ctxt_alloc_init()),
    softq_(mpscq_t::alloc(MAX_SOFTQ_SLOTS)),
    ctx_(NULL),
    feature_state_(NULL),
    feature_state_size_(0),
    num_features_(0),
    iflow_(NULL),
    rflow_(NULL)
{
    hal_ret_t ret;

    ret = cpupkt_register_rx_queue(arm_ctx_, types::WRING_TYPE_ARQRX, fte_id);
    HAL_ASSERT(ret == HAL_RET_OK);

    ret = cpupkt_register_rx_queue(arm_ctx_, types::WRING_TYPE_ARQTX, fte_id);
    HAL_ASSERT(ret == HAL_RET_OK);

    ret = cpupkt_register_tx_queue(arm_ctx_, types::WRING_TYPE_ASQ, fte_id);
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
    while(true) {
        if (hal_cfg->platform_mode == hal::HAL_PLATFORM_MODE_SIM) {
            usleep(1000000/3);
        } else if (hal_cfg->platform_mode == hal::HAL_PLATFORM_MODE_RTL) {
            usleep(1000000 * 3);
        }
        ctx_mem_init();
        process_arq();
        process_softq();
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
    return hal::pd::cpupkt_send(arm_ctx_, types::WRING_TYPE_ASQ, id_,
                                cpu_header, p4plus_header, pkt, pkt_len,
                                hal::SERVICE_LIF_CPU, CPU_ASQ_QTYPE,
                                id_, CPU_SCHED_RING_ASQ);
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
    ret = hal::pd::cpupkt_poll_receive(arm_ctx_, &cpu_rxhdr, &pkt, &pkt_len);
    if (ret == HAL_RET_RETRY) {
        return;
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: arm rx failed, ret={}", ret);
        return;
    }

    HAL_TRACE_DEBUG("fte: rxpkt cpu_rxhdr={}",
                    hex_str((uint8_t*)cpu_rxhdr, sizeof(*cpu_rxhdr)));

    HAL_TRACE_DEBUG("fte: rxpkt len={} pkt={}", pkt_len, hex_str(pkt, pkt_len));

    HAL_TRACE_DEBUG("fte: rxpkt slif={} lif={} qtype={} qid={} vrf={} "
                    "pad={} lkp_dir={} lkp_inst={} lkp_type={} flags={} "
                    "l2={} l3={} l4={} payload={}",
                    cpu_rxhdr->src_lif, cpu_rxhdr->lif, cpu_rxhdr->qtype,
                    cpu_rxhdr->qid, cpu_rxhdr->lkp_vrf, cpu_rxhdr->pad,
                    cpu_rxhdr->lkp_dir, cpu_rxhdr->lkp_inst, cpu_rxhdr->lkp_type,
                    cpu_rxhdr->flags, cpu_rxhdr->l2_offset, cpu_rxhdr->l3_offset,
                    cpu_rxhdr->l4_offset, cpu_rxhdr->payload_offset);

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
        hal::app_redir::app_redir_ctx(*ctx_, false)->set_arm_ctx(arm_ctx_);
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

} //   namespace fte
