//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "dpkt_learn_trans.hpp"

#include "nic/include/base.hpp"
#include "nic/utils/fsm/fsm.hpp"
#include "nic/hal/plugins/eplearn/eplearn.hpp"

using hal::utils::fsm_transition_t;
using hal::utils::fsm_transition_func;
using hal::utils::fsm_state_func;
using hal::utils::fsm_state_machine_def_t;
using hal::utils::fsm_state_machine_t;

/* Once all sessions are deleted, wait until we remove the entry from EP */
#define DPKT_LEARN_ENTRY_DELETE_TIMEOUT 120

namespace hal {

namespace eplearn {

uint32_t dpkt_learn_entry_delete_timeout = DPKT_LEARN_ENTRY_DELETE_TIMEOUT;

void *dpkt_learn_trans_get_key_func(void *entry) {
    HAL_ASSERT(entry != NULL);
    return (void *)(((dpkt_learn_trans_t *)entry)->trans_key_ptr());
}

uint32_t dpkt_learn_trans_compute_hash_func(void *key, uint32_t ht_size) {
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(dpkt_learn_trans_key_t)) % ht_size;
}

void dpkt_learn_entry_delete_timeout_set(uint32_t timeout) {
    dpkt_learn_entry_delete_timeout = timeout;
}

bool dpkt_learn_trans_compare_key_func(void *key1, void *key2) {
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (memcmp(key1, key2, sizeof(dpkt_learn_trans_key_t)) == 0) {
        return true;
    }
    return false;
}

/* Make sure only one instance of State machine is present and all transactions
 * should use the same state machine */
dpkt_learn_trans_t::dpkt_learn_fsm_t *dpkt_learn_trans_t::dpkt_learn_fsm_ = new dpkt_learn_fsm_t();
dpkt_learn_trans_t::trans_timer_t *dpkt_learn_trans_t::dpkt_learn_timer_ =
        new trans_timer_t(DPKT_LEARN_TIMER_ID);
slab *dpkt_learn_trans_t::dpkt_learn_slab_ =
    slab::factory("dpkt_learn", HAL_SLAB_DPKT_LEARN,
                  sizeof(dpkt_learn_trans_t), 16, false,
                  true, true);
ht *dpkt_learn_trans_t::dpkt_learn_key_ht_ =
    ht::factory(HAL_MAX_DPKT_LEARN_TRANS, dpkt_learn_trans_get_key_func,
                dpkt_learn_trans_compute_hash_func,
                dpkt_learn_trans_compare_key_func);
ht *dpkt_learn_trans_t::dpkt_learn_ip_entry_ht_ =
    ht::factory(HAL_MAX_DPKT_LEARN_TRANS,
                trans_get_ip_entry_key_func,
                trans_compute_ip_entry_hash_func,
                trans_compare_ip_entry_key_func);

// This is default, entry func can override this.
#define BOUND_TIMEOUT      60 *  (TIME_MSECS_PER_MIN)

// clang-format off
void dpkt_learn_trans_t::dpkt_learn_fsm_t::_init_state_machine() {
#define SM_FUNC(__func) SM_BIND_NON_STATIC(dpkt_learn_fsm_t, __func)
#define SM_FUNC_ARG_1(__func) SM_BIND_NON_STATIC_ARGS_1(dpkt_learn_fsm_t, __func)
    FSM_SM_BEGIN((sm_def))
        FSM_STATE_BEGIN(DPKT_LEARN_INIT, 0, NULL, NULL)
            FSM_TRANSITION(DPKT_LEARN_ADD, SM_FUNC(process_dpkt_learn_request), DPKT_LEARN_BOUND)
            FSM_TRANSITION(DPKT_LEARN_ERROR, NULL, DPKT_LEARN_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(DPKT_LEARN_BOUND, 0, NULL, NULL)
            FSM_TRANSITION(DPKT_IP_ADD, SM_FUNC(add_ip_entry), DPKT_LEARN_BOUND)
            FSM_TRANSITION(DPKT_LEARN_SESSIONS_DELETED, SM_FUNC(process_sessions_deleted), DPKT_LEARN_WAIT_FOR_DELETE)
            FSM_TRANSITION(DPKT_LEARN_REMOVE, NULL, DPKT_LEARN_DONE)
            FSM_TRANSITION(DPKT_LEARN_ERROR, NULL, DPKT_LEARN_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(DPKT_LEARN_WAIT_FOR_DELETE, 0, NULL, NULL)
            FSM_TRANSITION(DPKT_LEARN_REMOVE, NULL, DPKT_LEARN_DONE)
            FSM_TRANSITION(DPKT_LEARN_TIMEOUT, SM_FUNC(process_dpkt_learn_timeout), DPKT_LEARN_DONE)
            FSM_TRANSITION(DPKT_LEARN_REBOUND, NULL, DPKT_LEARN_BOUND)
            FSM_TRANSITION(DPKT_LEARN_ERROR, NULL, DPKT_LEARN_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(DPKT_LEARN_DONE, 0, NULL, NULL)
        FSM_STATE_END
    FSM_SM_END
    this->set_state_machine(sm_def);
}
// clang-format on


#define ADD_COMPLETION_HANDLER(__trans, __event, __ep_handle, __ip_addr)                      \
    uint32_t __trans_cnt = eplearn_info->trans_ctx_cnt;                                       \
    eplearn_info->trans_ctx[__trans_cnt].trans = __trans;                                     \
    eplearn_info->trans_ctx[__trans_cnt].event = __event;                                      \
    eplearn_info->trans_ctx[__trans_cnt].event_data.dpkt_learn_data.ep_handle = (__ep_handle); \
    eplearn_info->trans_ctx[__trans_cnt].event_data.dpkt_learn_data.ip_addr =  (__ip_addr);    \
    eplearn_info->trans_ctx_cnt++;                                                              \
    fte_ctx->register_completion_handler(trans_t::trans_completion_handler);

static void dpkt_timeout_handler(void *timer, uint32_t timer_id, void *ctxt) {
    fsm_state_machine_t* sm_ = reinterpret_cast<fsm_state_machine_t*>(ctxt);
    sm_->reset_timer();
    trans_t* trans =
        reinterpret_cast<trans_t*>(sm_->get_ctx());
    trans->log_info("Received Data packet learn timeout event.");
    trans_t::process_transaction(trans, DPKT_LEARN_TIMEOUT, NULL);
}


ep_t* dpkt_learn_trans_t::get_ep_entry() {
    ep_t *other_ep_entry = NULL;
    ep_l3_key_t l3_key = {0};

    l3_key.vrf_id = this->trans_key_ptr()->vrf_id;
    memcpy(&l3_key.ip_addr, &this->ip_addr_, sizeof(ip_addr_t));

    /* Find the EP entry for this IP address */
    other_ep_entry = find_ep_by_l3_key(&l3_key);

    return other_ep_entry;
}

bool dpkt_learn_trans_t::dpkt_learn_fsm_t::add_ip_entry(fsm_state_ctx ctx,
                                          fsm_event_data fsm_data)
{
    dpkt_learn_trans_t *trans = reinterpret_cast<dpkt_learn_trans_t *>(ctx);
    dpkt_learn_event_data_t *data = reinterpret_cast<dpkt_learn_event_data_t*>(fsm_data);
    hal_ret_t ret;

    ret = eplearn_ip_move_process(data->ep_handle,
            &trans->ip_entry_key_ptr()->ip_addr, DPKT_LEARN);

    if (ret != HAL_RET_OK) {
       trans->log_error("IP move process failed, skipping IP add.");
       trans->sm_->throw_event(DPKT_LEARN_ERROR, NULL);
       return false;
    }

    ep_t *ep_entry = find_ep_by_handle(data->ep_handle);
    trans->log_info("Trying to add IP to EP entry.");
    ret = endpoint_update_ip_add(ep_entry,
            &trans->ip_entry_key_ptr()->ip_addr, EP_FLAGS_LEARN_SRC_FLOW_MISS);

    if (ret != HAL_RET_OK) {
       trans->log_error("IP add update failed");
       trans->sm_->throw_event(DPKT_LEARN_ERROR, NULL);
       return false;
    }

    trans->log_info("Successfully added IP to EP entry.");
    dpkt_learn_trans_t::dpkt_learn_key_ht()->insert((void *)trans, &trans->ht_ctxt_);
    dpkt_learn_trans_t::dpkt_learn_ip_entry_ht()->insert((void *)trans,
                                                &trans->ip_entry_ht_ctxt_);


    return true;
}

bool dpkt_learn_trans_t::dpkt_learn_fsm_t::del_ip_entry(fsm_state_ctx ctx,
        fsm_event_data fsm_data)
{
    dpkt_learn_trans_t *trans = reinterpret_cast<dpkt_learn_trans_t *>(ctx);
    dpkt_learn_event_data_t *data = reinterpret_cast<dpkt_learn_event_data_t*>(fsm_data);
    hal_ret_t ret;
    ep_t *ep_entry = find_ep_by_handle(data->ep_handle);

    if (ep_entry != NULL) {
        ret = endpoint_update_ip_delete(ep_entry,
                &trans->ip_entry_key_ptr()->ip_addr, EP_FLAGS_LEARN_SRC_FLOW_MISS);
        if (ret != HAL_RET_OK) {
            trans->log_error("IP delete update failed");
        }
        ep_entry = trans->get_ep_entry();
        HAL_ASSERT(ep_entry == NULL);
    }

    return true;
}

bool dpkt_learn_trans_t::dpkt_learn_fsm_t::process_dpkt_learn_request(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    dpkt_learn_trans_t *trans = reinterpret_cast<dpkt_learn_trans_t *>(ctx);
    dpkt_learn_event_data_t *data = reinterpret_cast<dpkt_learn_event_data_t*>(fsm_data);
    fte::ctx_t *fte_ctx = data->fte_ctx;
    const ip_addr_t *ip_addr = &data->ip_addr;
    hal_handle_t ep_handle = fte_ctx->sep_handle();
    ep_t *ep_entry;
    eplearn_info_t *eplearn_info = (eplearn_info_t*)\
                fte_ctx->feature_state(FTE_FEATURE_EP_LEARN);

    trans->log_info("Processing Flow miss learn request.");
    ep_entry = find_ep_by_handle(ep_handle);
    if (ep_entry == nullptr) {
        trans->log_error("Endpoint entry not found.");
        trans->sm_->throw_event(DPKT_LEARN_ERROR, NULL);
        return false;
    }


    /* Setup IP entry key for new transaction first */
    trans->set_up_ip_entry_key(ip_addr);

    dpkt_learn_trans_t *other_trans = reinterpret_cast<dpkt_learn_trans_t *>(
        dpkt_learn_trans_t::dpkt_learn_ip_entry_ht()->lookup(
                (trans->ip_entry_key_ptr())));

    if (other_trans != NULL) {
        trans->log_error("Found an existing transaction, will be removed.");
        other_trans->log_error("Initiating transaction delete.");
        dpkt_learn_trans_t::process_transaction(other_trans, DPKT_LEARN_REMOVE, NULL);
    }

    if(data->in_fte_pipeline) {
        /* Everything looks good, do a commit after FTE pipeline completed. */
        ADD_COMPLETION_HANDLER(trans, DPKT_IP_ADD,
                ep_entry->hal_handle, *ip_addr);
    } else {
        add_ip_entry(ctx, fsm_data);
    }

    return true;
}



bool dpkt_learn_trans_t::dpkt_learn_fsm_t::process_dpkt_learn_timeout(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data)
{
    dpkt_learn_trans_t *trans = reinterpret_cast<dpkt_learn_trans_t *>(ctx);

    /* TOOD : Need an API to check whether sessions with this IP and EP exists.
     *  If exists, should not delete this entry.
     *  */
    trans->dpkt_learn_timer_ctx = nullptr;

    return true;
}


bool dpkt_learn_trans_t::dpkt_learn_fsm_t::process_sessions_deleted(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    dpkt_learn_trans_t *trans = reinterpret_cast<dpkt_learn_trans_t *>(ctx);

    trans->dpkt_learn_timer_ctx = nullptr;


    HAL_TRACE_INFO("Starting Data pkt learn timeout of {} seconds", dpkt_learn_entry_delete_timeout);
    trans->dpkt_learn_timer_ctx =
            dpkt_learn_timer_->add_timer_with_custom_handler(
                    dpkt_learn_entry_delete_timeout * TIME_MSECS_PER_SEC,
            trans->sm_, dpkt_timeout_handler);

    return true;
}


void dpkt_learn_trans_t::set_up_ip_entry_key(const ip_addr_t *ip_addr) {
    memcpy(&this->ip_addr_, ip_addr, sizeof(ip_addr_t));
    init_ip_entry_key(ip_addr, this->trans_key_ptr()->vrf_id,
            this->ip_entry_key_ptr());
}

bool dpkt_learn_trans_t::protocol_address_match(const ip_addr_t *ip_addr) {
    return (!memcmp(&this->ip_addr_, ip_addr, sizeof(ip_addr_t)));
}

void *dpkt_learn_trans_t::operator new(size_t size) {
    void *p = dpkt_learn_trans_alloc_init();
    return p;
}

void dpkt_learn_trans_t::operator delete(void *p) {
    dpkt_learn_trans_t *trans = reinterpret_cast<dpkt_learn_trans_t *>(p);
    dpkt_learn_trans_free(trans);
}

dpkt_learn_trans_t::dpkt_learn_trans_t(dpkt_learn_trans_key_t *trans_key,  fte::ctx_t &ctx) {

    memcpy(&this->trans_key_, trans_key, sizeof(dpkt_learn_trans_key_t));
    this->sm_ = new fsm_state_machine_t(get_sm_def_func, DPKT_LEARN_INIT, DPKT_LEARN_DONE,
                                        DPKT_LEARN_TIMEOUT, DPKT_LEARN_REMOVE,
                                        (fsm_state_ctx)this,
                                        get_timer_func);
    this->ht_ctxt_.reset();
    this->ip_entry_ht_ctxt_.reset();
}

void dpkt_learn_trans_t::process_event(dpkt_learn_fsm_event_t event, fsm_event_data data) {
    this->sm_->process_event(event, data);
}

dpkt_learn_trans_t::~dpkt_learn_trans_t() {
    this->log_info("Deleting transaction.");
    this->reset();
    delete this->sm_;
}

void dpkt_learn_trans_t::reset() {
    hal_ret_t ret;
    ep_t * ep_entry;

    if (this->dpkt_learn_timer_ctx) {
        dpkt_learn_timer_->delete_timer(this->dpkt_learn_timer_ctx);
        this->dpkt_learn_timer_ctx = nullptr;
    }
    dpkt_learn_trans_t::dpkt_learn_key_ht()->remove(&this->trans_key_);
    dpkt_learn_trans_t::dpkt_learn_ip_entry_ht()->remove(this->ip_entry_key_ptr());
    /*
     * TODO : Also remove the IP entry from the associated EP as well.
     */

    ep_entry = this->get_ep_entry();
    if (ep_entry != NULL) {
        ret = endpoint_update_ip_delete(ep_entry,
                &this->ip_entry_key_ptr()->ip_addr, EP_FLAGS_LEARN_SRC_FLOW_MISS);
        if (ret != HAL_RET_OK) {
            this->log_error("IP delete update failed");
        }
    }

    this->sm_->stop_state_timer();
}



void dpkt_learn_trans_t::init_dpkt_learn_trans_key(const uint8_t *hw_addr, const ep_t *ep,
                                        ip_addr_t *ip_addr, dpkt_learn_trans_key_t *trans_key) {
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
}

dpkt_learn_fsm_state_t dpkt_learn_trans_t::get_state() {
    return (dpkt_learn_fsm_state_t)this->sm_->get_state();
}

hal_ret_t
dpkt_learn_process_ip_move(hal_handle_t ep_handle, const ip_addr_t *ip_addr) {

    return dpkt_learn_trans_t::process_ip_move(ep_handle, ip_addr,
            dpkt_learn_trans_t::get_ip_ht());
}

}  // namespace eplearn
}  // namespace hal
