//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <arpa/inet.h>
#include "nic/include/base.hpp"
#include "arp_trans.hpp"
#include "nic/utils/fsm/fsm.hpp"
#include "nic/hal/plugins/eplearn/eplearn.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "nic/fte/utils/packet_utils.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "nic/include/pd_api.hpp"

using hal::utils::fsm_transition_t;
using hal::utils::fsm_transition_func;
using hal::utils::fsm_state_func;
using hal::utils::fsm_state_machine_def_t;
using hal::utils::fsm_state_machine_t;

namespace hal {

namespace eplearn {

#define ARP_PROBE_TIMEOUT  3 * (TIME_MSECS_PER_SEC)

void *arptrans_get_key_func(void *entry) {
    SDK_ASSERT(entry != NULL);
    return (void *)(((arp_trans_t *)entry)->trans_key_ptr());
}

uint32_t arptrans_key_size(void) {
    return sizeof(arp_trans_key_t);
}

/* Make sure only one instance of State machine is present and all transactions
 * should use the same state machine */
arp_trans_t::arp_fsm_t *arp_trans_t::arp_fsm_ = new arp_fsm_t();
arp_trans_t::trans_timer_t *arp_trans_t::arp_timer_ =
        new trans_timer_t(ARP_TIMER_ID);
uint32_t arp_trans_t::arp_probe_timeout = ARP_PROBE_TIMEOUT;
slab *arp_trans_t::arplearn_slab_ =
    slab::factory("arpLearn", HAL_SLAB_ARP_LEARN,
                  sizeof(arp_trans_t), 16, false,
                  true, true);
ht *arp_trans_t::arplearn_key_ht_ =
    ht::factory(HAL_MAX_ARP_TRANS,
                arptrans_get_key_func,
                arptrans_key_size());
ht *arp_trans_t::arplearn_ip_entry_ht_ =
    ht::factory(HAL_MAX_ARP_TRANS,
                trans_get_ip_entry_key_func,
                trans_ip_entry_key_size());

// This is default, entry func can override this.
#define BOUND_TIMEOUT      60 *  (TIME_MSECS_PER_MIN)


// clang-format off
void arp_trans_t::arp_fsm_t::_init_state_machine() {
#define SM_FUNC(__func) SM_BIND_NON_STATIC(arp_fsm_t, __func)
#define SM_FUNC_ARG_1(__func) SM_BIND_NON_STATIC_ARGS_1(arp_fsm_t, __func)
    FSM_SM_BEGIN((sm_def))
        FSM_STATE_BEGIN(ARP_INIT, 0, NULL, NULL)
            FSM_TRANSITION(ARP_ADD, SM_FUNC(process_arp_request), ARP_BOUND)
            FSM_TRANSITION(RARP_REQ, SM_FUNC(process_rarp_request), RARP_INIT)
            FSM_TRANSITION(ARP_ERROR, NULL, ARP_DONE)
            FSM_TRANSITION(ARP_DUPLICATE, NULL, ARP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(RARP_INIT, 0, NULL, NULL)
            FSM_TRANSITION(RARP_REPLY, SM_FUNC(process_rarp_reply),
                           ARP_BOUND)
            FSM_TRANSITION(ARP_TIMEOUT, SM_FUNC(process_arp_timeout), ARP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(ARP_BOUND, 0, NULL, NULL)
            FSM_TRANSITION(ARP_ADD, SM_FUNC(process_arp_renewal_request),
                           ARP_BOUND)
            FSM_TRANSITION(ARP_IP_RESET_ADD, SM_FUNC(reset_and_add_new_ip),
                           ARP_BOUND)
            FSM_TRANSITION(ARP_IP_ADD, SM_FUNC(add_ip_entry), ARP_BOUND)
            FSM_TRANSITION(ARP_TIMEOUT, SM_FUNC(process_arp_timeout), ARP_DONE)
            FSM_TRANSITION(ARP_PROBE_SENT, NULL, ARP_PROBING)
            FSM_TRANSITION(ARP_ERROR, NULL, ARP_DONE)
            FSM_TRANSITION(ARP_DUPLICATE, NULL, ARP_DONE)
            FSM_TRANSITION(ARP_REMOVE, NULL, ARP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(ARP_PROBING, 0, NULL, NULL)
            FSM_TRANSITION(ARP_TIMEOUT, SM_FUNC(process_arp_probe_timeout), ARP_DONE)
            FSM_TRANSITION(ARP_ADD, SM_FUNC(process_arp_renewal_request),
                           ARP_BOUND)
            FSM_TRANSITION(ARP_DUPLICATE, NULL, ARP_DONE)
            FSM_TRANSITION(ARP_REMOVE, NULL, ARP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(ARP_DONE, 0, NULL, NULL)
        FSM_STATE_END
    FSM_SM_END
    this->set_state_machine(sm_def);
}
// clang-format on


static void arp_timeout_handler(void *timer, uint32_t timer_id, void *ctxt) {
    fsm_state_machine_t* sm_ = reinterpret_cast<fsm_state_machine_t*>(ctxt);
    sm_->reset_timer();
    trans_t* trans =
        reinterpret_cast<trans_t*>(sm_->get_ctx());
    trans->log_info("Received ARP timeout event.");
    trans_t::process_transaction(trans, ARP_TIMEOUT, NULL);
}

#define ADD_COMPLETION_HANDLER(__trans, __event, __ep_handle, __ip_addr)               \
    uint32_t __trans_cnt = eplearn_info->trans_ctx_cnt;                                \
    eplearn_info->trans_ctx[__trans_cnt].trans = __trans;                              \
    eplearn_info->trans_ctx[__trans_cnt].event = __event;                              \
    eplearn_info->trans_ctx[__trans_cnt].event_data.arp_data.ep_handle = (__ep_handle); \
    eplearn_info->trans_ctx[__trans_cnt].event_data.arp_data.ip_addr =  (__ip_addr);    \
    eplearn_info->trans_ctx[__trans_cnt].event_data.arp_data.in_fte_pipeline = false;   \
    eplearn_info->trans_ctx_cnt++;                                                      \
    fte_ctx->register_completion_handler(trans_t::trans_completion_handler);

ep_t* arp_trans_t::get_ep_entry() {
    ep_t *other_ep_entry = NULL;
    ep_l3_key_t l3_key = {0};

    l3_key.vrf_id = this->trans_key_ptr()->vrf_id;
    memcpy(&l3_key.ip_addr, &this->ip_addr_, sizeof(ip_addr_t));

    /* Find the EP entry for this IP address */
    other_ep_entry = find_ep_by_l3_key(&l3_key);

    return other_ep_entry;
}

bool arp_trans_t::arp_fsm_t::add_ip_entry(fsm_state_ctx ctx,
                                          fsm_event_data fsm_data)
{
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(ctx);
    arp_event_data_t *data = reinterpret_cast<arp_event_data_t*>(fsm_data);
    hal_ret_t ret;

    ret = eplearn_ip_move_process(data->ep_handle,
            &trans->ip_entry_key_ptr()->ip_addr, ARP_LEARN);

    if (ret != HAL_RET_OK) {
       trans->log_error("IP move process failed, skipping IP add.");
       trans->sm_->throw_event(ARP_ERROR, NULL);
       return false;
    }

    ep_t *ep_entry = find_ep_by_handle(data->ep_handle);
    trans->log_info("Trying to add IP to EP entry.");
    ret = endpoint_update_ip_add(ep_entry,
            &trans->ip_entry_key_ptr()->ip_addr, EP_FLAGS_LEARN_SRC_ARP);

    if (ret != HAL_RET_OK) {
       trans->log_error("IP add update failed");
       trans->sm_->throw_event(ARP_ERROR, NULL);
       /* We should probably drop this packet as this ARP request may not be valid one */
       return false;
    }

    trans->log_info("Successfully added IP to EP entry.");
    arp_trans_t::arplearn_key_ht()->insert((void *)trans, &trans->ht_ctxt_);
    arp_trans_t::arplearn_ip_entry_ht()->insert((void *)trans,
                                                &trans->ip_entry_ht_ctxt_);

    trans->start_arp_timer();

    return true;
}

bool arp_trans_t::arp_fsm_t::del_ip_entry(fsm_state_ctx ctx,
        fsm_event_data fsm_data)
{
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(ctx);
    arp_event_data_t *data = reinterpret_cast<arp_event_data_t*>(fsm_data);
    hal_ret_t ret;
    ep_t *ep_entry = find_ep_by_handle(data->ep_handle);

    if (ep_entry != NULL) {
        ret = endpoint_update_ip_delete(ep_entry,
                &trans->ip_entry_key_ptr()->ip_addr, EP_FLAGS_LEARN_SRC_ARP);
        if (ret != HAL_RET_OK) {
            trans->log_error("IP delete update failed");
        }
        ep_entry = trans->get_ep_entry();
        SDK_ASSERT(ep_entry == NULL);
    }

    return true;
}

bool arp_trans_t::arp_fsm_t::process_arp_request(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(ctx);
    arp_event_data_t *data = reinterpret_cast<arp_event_data_t*>(fsm_data);
    fte::ctx_t *fte_ctx = data->fte_ctx;
    const ip_addr_t *ip_addr = &data->ip_addr;
    hal_handle_t ep_handle = fte_ctx->sep_handle();
    ep_t *ep_entry;
    eplearn_info_t *eplearn_info = (eplearn_info_t*)\
                fte_ctx->feature_state(FTE_FEATURE_EP_LEARN);

    trans->log_info("Processing ARP request.");
    ep_entry = find_ep_by_handle(ep_handle);
    if (ep_entry == nullptr) {
        trans->log_error("Endpoint entry not found.");
        trans->sm_->throw_event(ARP_ERROR, NULL);
        return false;
    }


    /* Setup IP entry key for new transaction first */
    trans->set_up_ip_entry_key(ip_addr);

    arp_trans_t *other_trans = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_ip_entry_ht()->lookup(
                (trans->ip_entry_key_ptr())));

    if (other_trans != NULL) {
        trans->log_error("Found an existing transaction, will be removed.");
        other_trans->log_error("Initiating transaction delete.");
        arp_trans_t::process_transaction(other_trans, ARP_REMOVE, NULL);
    }

    if(data->in_fte_pipeline) {
        /* Everything looks good, do a commit after FTE pipeline completed. */
        ADD_COMPLETION_HANDLER(trans, ARP_IP_ADD,
                ep_entry->hal_handle, *ip_addr);
    } else {
        add_ip_entry(ctx, fsm_data);
    }

    return true;
}

bool arp_trans_t::arp_fsm_t::process_rarp_reply(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(ctx);
    arp_event_data_t *data = reinterpret_cast<arp_event_data_t*>(fsm_data);
    fte::ctx_t *fte_ctx = data->fte_ctx;
    const ip_addr_t *ip_addr = &data->ip_addr;
    hal_handle_t ep_handle = fte_ctx->dep_handle();
    ep_t *ep_entry;
    eplearn_info_t *eplearn_info = (eplearn_info_t*)\
                fte_ctx->feature_state(FTE_FEATURE_EP_LEARN);

    trans->log_info("Processing RARP reply.");
    ep_entry = find_ep_by_handle(ep_handle);
    if (ep_entry == nullptr) {
        trans->log_error("Endpoint entry not found.");
        trans->sm_->throw_event(ARP_ERROR, NULL);
        return false;
    }

    /* Setup IP entry key for new transaction first */
    trans->set_up_ip_entry_key(ip_addr);

    arp_trans_t *other_trans = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_ip_entry_ht()->lookup(
                (trans->ip_entry_key_ptr())));

    if (other_trans != NULL) {
        trans->log_error("Found an existing transaction, will be removed.");
        other_trans->log_error("Initiating transaction delete.");
        arp_trans_t::process_transaction(other_trans, ARP_REMOVE, NULL);
    }

    /* Everything looks good, do a commit after FTE pipeline completed. */

    ADD_COMPLETION_HANDLER(trans, ARP_IP_ADD,
                ep_entry->hal_handle, *ip_addr);

    return true;
}

bool arp_trans_t::arp_fsm_t::process_rarp_request(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(ctx);
    arp_event_data_t *data = reinterpret_cast<arp_event_data_t*>(fsm_data);
    fte::ctx_t *fte_ctx = data->fte_ctx;
    hal_handle_t ep_handle = fte_ctx->sep_handle();
    ep_t *ep_entry;

    trans->log_info("Processing RARP request.");
    ep_entry = find_ep_by_handle(ep_handle);
    if (ep_entry == nullptr) {
        trans->log_error("Endpoint entry not found.");
        trans->sm_->throw_event(ARP_ERROR, NULL);
        return false;
    }
    // handover to vmotion statemachine in following scenarios
    // - rarp rcvd after ep update recevied by proto (vmotion_state)
    // - rarp rcvd first, so ep points to non_local
    //   add check for src_intf being eNIC (DMA)
    if ((endpoint_is_remote(ep_entry)) &&
        (fte_ctx->cpu_rxhdr()->lkp_dir == hal::FLOW_DIR_FROM_DMA) &&
        (g_hal_state->get_vmotion())) {
        // TODO: add a debug counter and increment it to keep track of rarp pkts
        // drop the original rarp packet
        // will be re-created from EP and sent after sync complete 
        if (hal::g_hal_state->get_vmotion()->process_rarp(*(ep_get_mac_addr(ep_entry))))
            fte_ctx->set_drop();
    } else {
        arp_trans_t::arplearn_key_ht()->insert((void *)trans, &trans->ht_ctxt_);
    }

    return true;
}

struct arp_probe_ctx_t {
    arp_trans_t    *trans;
    hal_ret_t       ret;
    bool            sent;
};


static void
send_arp_probe_request(void *data) {
    arp_probe_ctx_t *arp_ctx = reinterpret_cast<arp_probe_ctx_t*>(data);
    hal::l2seg_t   *l2seg;
    ip_addr_t       zero_ip = { 0 };
    ep_t*           ep = arp_ctx->trans->get_ep_entry();
    fte::utils::arp_pkt_data_t pkt_data = { 0 };

    arp_ctx->sent = false;
    arp_ctx->ret = HAL_RET_OK;

    if (ep == nullptr) {
        arp_ctx->trans->log_info("EP not found to send ARP probe");
        return;
    }

    l2seg = find_l2seg_by_id(arp_ctx->trans->trans_key_ptr()->l2_segid);
    if (l2seg != nullptr && l2seg->eplearn_cfg.arp_cfg.enabled &&
            l2seg->eplearn_cfg.arp_cfg.probe_enabled) {
        arp_ctx->trans->log_info("Trying to send Probing ARP request");
        pkt_data.ep = ep;
        pkt_data.src_ip_addr = &zero_ip;
        pkt_data.src_mac = g_hal_state->get_local_mac_address();
        pkt_data.dst_ip_addr = &(arp_ctx->trans->trans_key_ptr()->ip_addr);
        arp_ctx->ret = fte::utils::hal_inject_arp_request_pkt(&pkt_data);
        if (arp_ctx->ret == HAL_RET_OK) {
            arp_ctx->sent = true;
        }
    }
}

bool arp_trans_t::arp_fsm_t::process_arp_timeout(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    arp_trans_t    *trans = reinterpret_cast<arp_trans_t *>(ctx);
    trans->arp_timer_ctx = nullptr;
    struct arp_probe_ctx_t *fn_ctx = (struct arp_probe_ctx_t*)
            HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE, sizeof(struct arp_probe_ctx_t));
    bool ret = true;
    fn_ctx->trans = trans;
    fte::fte_execute(0, send_arp_probe_request, fn_ctx);

    if (fn_ctx->ret == HAL_RET_OK) {
        if (fn_ctx->sent) {
             HAL_TRACE_INFO("Starting ARP Probe timeout {} milli seconds",
                     trans->arp_probe_timeout);
              trans->arp_timer_ctx =
                      arp_timer_->add_timer_with_custom_handler(
                              trans->arp_probe_timeout,
                              trans->sm_, arp_timeout_handler);
             /* Successfully sent ARP probe request, wait for ARP probe*/
             trans->sm_->throw_event(ARP_PROBE_SENT, NULL);
             ret = false;
             goto cleanup;
        } else {
            trans->log_error("Arp probe request not sent");
        }
    } else {
        trans->log_error("Error in sending arp probe request..");
    }

cleanup:
    HAL_FREE(hal::HAL_MEM_ALLOC_FTE, fn_ctx);
    return ret;
}

bool arp_trans_t::arp_fsm_t::process_arp_probe_timeout(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data)
{
    arp_trans_t    *trans = reinterpret_cast<arp_trans_t *>(ctx);

    trans->arp_timer_ctx = nullptr;

    return true;
}


bool arp_trans_t::arp_fsm_t::process_arp_renewal_request(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(ctx);
    arp_event_data_t *data = reinterpret_cast<arp_event_data_t*>(fsm_data);
    fte::ctx_t *fte_ctx = data->fte_ctx;
    const ip_addr_t *ip_addr = &data->ip_addr;
    hal_handle_t ep_handle = fte_ctx->sep_handle();
    ep_t *ep_entry;
    eplearn_info_t *eplearn_info = (eplearn_info_t*)\
                fte_ctx->feature_state(FTE_FEATURE_EP_LEARN);
    bool ret = true;

    ep_entry = find_ep_by_handle(ep_handle);
    trans->log_info("Processing ARP renewal request.");
    if (ep_entry == nullptr) {
        trans->log_error("Endpoint entry not found.");
        trans->sm_->throw_event(ARP_ERROR, NULL);
        return false;
    }

    if (trans->protocol_address_match(ip_addr)) {
        /* Just a renewal, nothing to do, timeout will be updated. */
        trans->log_info("Protocol address same, nothing to do.");
        trans->start_arp_timer();
        return true;
    }

    if (data->in_fte_pipeline) {
        ADD_COMPLETION_HANDLER(trans, ARP_IP_RESET_ADD,
                    ep_entry->hal_handle, *ip_addr);
    } else {
        ret = reset_and_add_new_ip(ctx, fsm_data);
    }

    return ret;
}

bool arp_trans_t::arp_fsm_t::reset_and_add_new_ip(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(ctx);

    trans->log_info("Resetting and Adding a new IP to transaction.");

    trans->reset();
    return process_arp_request(ctx, fsm_data);
}

void arp_trans_t::set_up_ip_entry_key(const ip_addr_t *ip_addr) {
    memcpy(&this->ip_addr_, ip_addr, sizeof(ip_addr_t));
    init_ip_entry_key(ip_addr, this->trans_key_ptr()->vrf_id,
            this->ip_entry_key_ptr());
}

bool arp_trans_t::protocol_address_match(const ip_addr_t *ip_addr) {
    return (!memcmp(&this->ip_addr_, ip_addr, sizeof(ip_addr_t)));
}

void *arp_trans_t::operator new(size_t size) {
    void *p = arp_trans_alloc_init();
    return p;
}

void arp_trans_t::operator delete(void *p) {
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(p);
    arp_trans_free(trans);
}

arp_trans_t::arp_trans_t(arp_trans_key_t *trans_key, fte::ctx_t &ctx) {
    memcpy(&this->trans_key_, trans_key, sizeof(arp_trans_key_t));
    this->sm_ = new fsm_state_machine_t(get_sm_def_func, ARP_INIT, ARP_DONE,
                                        ARP_TIMEOUT, ARP_REMOVE,
                                        (fsm_state_ctx)this,
                                        get_timer_func);
    this->arp_timer_ctx = nullptr;
    this->ht_ctxt_.reset();
    this->ip_entry_ht_ctxt_.reset();
}

void arp_trans_t::process_event(arp_fsm_event_t event, fsm_event_data data) {
    this->sm_->process_event(event, data);
}

arp_trans_t::~arp_trans_t() {
    this->log_info("Deleting transaction.");
    this->reset();
    delete this->sm_;
}

void arp_trans_t::reset() {
    hal_ret_t ret;
    ep_t * ep_entry;

    this->stop_arp_timer();
    arp_trans_t::arplearn_key_ht()->remove(&this->trans_key_);
    arp_trans_t::arplearn_ip_entry_ht()->remove(this->ip_entry_key_ptr());
    /*
     * TODO : Also remove the IP entry from the associated EP as well.
     */

    ep_entry = this->get_ep_entry();
    if (ep_entry != NULL) {
        ret = endpoint_update_ip_delete(ep_entry,
                &this->ip_entry_key_ptr()->ip_addr, EP_FLAGS_LEARN_SRC_ARP);
        if (ret != HAL_RET_OK) {
            this->log_error("IP delete update failed");
        }
    }

    this->sm_->stop_state_timer();
}


void arp_trans_t::start_arp_timer() {

    hal::l2seg_t *l2seg;
    uint32_t timeout;

    this->stop_arp_timer();

    l2seg = find_l2seg_by_id(this->trans_key_ptr()->l2_segid);
    if (l2seg != nullptr) {
        timeout = l2seg->eplearn_cfg.arp_cfg.entry_timeout;
    } else {
        HAL_TRACE_ERR("L2 segment look up failed, starting ARP timer with default time");
        timeout = BOUND_TIMEOUT;
    }


    HAL_TRACE_INFO("Starting ARP timer of {} seconds", timeout);
    this->arp_timer_ctx = arp_timer_->add_timer_with_custom_handler(
            timeout * TIME_MSECS_PER_SEC,
            this->sm_, arp_timeout_handler);
}

void arp_trans_t::stop_arp_timer() {

    if (this->arp_timer_ctx) {
        arp_timer_->delete_timer(this->arp_timer_ctx);
    }
    this->arp_timer_ctx = nullptr;
}

void arp_trans_t::init_arp_trans_key(const uint8_t *hw_addr, const ep_t *ep,
                                     arp_trans_type_t type, ip_addr_t *ip_addr,
                                     arp_trans_key_t *trans_key) {
    for (uint32_t i = 0; i < ETHER_ADDR_LEN; ++i) {
        trans_key->mac_addr[i] = (uint8_t)hw_addr[i];
    }
    trans_key->l2_segid = ep->l2_key.l2_segid;
    vrf_t *vrf = vrf_lookup_by_handle(ep->vrf_handle);
    if (vrf == NULL) {
        HAL_ABORT(0);
    }
    trans_key->vrf_id = vrf->vrf_id;
    trans_key->ip_addr = *ip_addr;
    trans_key->type = type;
}

arp_fsm_state_t arp_trans_t::get_state() {
    return (arp_fsm_state_t)this->sm_->get_state();
}


hal_ret_t
arp_process_ip_move(hal_handle_t ep_handle, const ip_addr_t *ip_addr) {

    return arp_trans_t::process_ip_move(ep_handle, ip_addr,
            arp_trans_t::get_ip_ht());
}

}  // namespace eplearn
}  // namespace hal
