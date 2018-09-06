//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "fmiss_learn_trans.hpp"
#include "nic/include/base.hpp"
#include "nic/utils/fsm/fsm.hpp"
#include "nic/hal/plugins/eplearn/eplearn.hpp"

using hal::utils::fsm_transition_t;
using hal::utils::fsm_transition_func;
using hal::utils::fsm_state_func;
using hal::utils::fsm_state_machine_def_t;
using hal::utils::fsm_state_machine_t;

namespace hal {

namespace eplearn {

void *fmiss_learn_trans_get_key_func(void *entry) {
    HAL_ASSERT(entry != NULL);
    return (void *)(((fmiss_learn_trans_t *)entry)->trans_key_ptr());
}

uint32_t fmiss_learn_trans_compute_hash_func(void *key, uint32_t ht_size) {
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(fmiss_learn_trans_key_t)) % ht_size;
}

bool fmiss_learn_trans_compare_key_func(void *key1, void *key2) {
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (memcmp(key1, key2, sizeof(fmiss_learn_trans_key_t)) == 0) {
        return true;
    }
    return false;
}

/* Make sure only one instance of State machine is present and all transactions
 * should use the same state machine */
fmiss_learn_trans_t::fmiss_learn_fsm_t *fmiss_learn_trans_t::fmiss_learn_fsm_ = new fmiss_learn_fsm_t();
fmiss_learn_trans_t::trans_timer_t *fmiss_learn_trans_t::fmiss_learn_timer_ =
        new trans_timer_t(FMISS_LEARN_TIMER_ID);
slab *fmiss_learn_trans_t::fmiss_learn_slab_ =
    slab::factory("fmiss_learn", HAL_SLAB_FMISS_LEARN,
                  sizeof(fmiss_learn_trans_t), 16, false,
                  true, true);
ht *fmiss_learn_trans_t::fmiss_learn_key_ht_ =
    ht::factory(HAL_MAX_FMISS_LEARN_TRANS, fmiss_learn_trans_get_key_func,
                fmiss_learn_trans_compute_hash_func,
                fmiss_learn_trans_compare_key_func);
ht *fmiss_learn_trans_t::fmiss_learn_ip_entry_ht_ =
    ht::factory(HAL_MAX_FMISS_LEARN_TRANS,
                trans_get_ip_entry_key_func,
                trans_compute_ip_entry_hash_func,
                trans_compare_ip_entry_key_func);

// This is default, entry func can override this.
#define BOUND_TIMEOUT      60 *  (TIME_MSECS_PER_MIN)

// clang-format off
void fmiss_learn_trans_t::fmiss_learn_fsm_t::_init_state_machine() {
#define SM_FUNC(__func) SM_BIND_NON_STATIC(fmiss_learn_fsm_t, __func)
#define SM_FUNC_ARG_1(__func) SM_BIND_NON_STATIC_ARGS_1(fmiss_learn_fsm_t, __func)
    FSM_SM_BEGIN((sm_def))
        FSM_STATE_BEGIN(FMISS_LEARN_INIT, 0, NULL, NULL)
            FSM_TRANSITION(FMISS_LEARN_ADD, SM_FUNC(process_fmiss_learn_request), FMISS_LEARN_BOUND)
            FSM_TRANSITION(FMISS_LEARN_ERROR, NULL, FMISS_LEARN_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(FMISS_LEARN_BOUND, 0, NULL, NULL)
            FSM_TRANSITION(FMISS_IP_ADD, SM_FUNC(add_ip_entry), FMISS_LEARN_BOUND)
            FSM_TRANSITION(FMISS_LEARN_TIMEOUT, SM_FUNC(process_fmiss_learn_timeout), FMISS_LEARN_DONE)
            FSM_TRANSITION(FMISS_LEARN_REMOVE, NULL, FMISS_LEARN_DONE)
            FSM_TRANSITION(FMISS_LEARN_ERROR, NULL, FMISS_LEARN_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(FMISS_LEARN_DONE, 0, NULL, NULL)
        FSM_STATE_END
    FSM_SM_END
    this->set_state_machine(sm_def);
}
// clang-format on


#define ADD_COMPLETION_HANDLER(__trans, __event, __ep_handle, __ip_addr)                      \
    uint32_t __trans_cnt = eplearn_info->trans_ctx_cnt;                                       \
    eplearn_info->trans_ctx[__trans_cnt].trans = __trans;                                     \
    eplearn_info->trans_ctx[__trans_cnt].event = __event;                                      \
    eplearn_info->trans_ctx[__trans_cnt].event_data.fmiss_learn_data.ep_handle = (__ep_handle); \
    eplearn_info->trans_ctx[__trans_cnt].event_data.fmiss_learn_data.ip_addr =  (__ip_addr);    \
    eplearn_info->trans_ctx_cnt++;                                                              \
    fte_ctx->register_completion_handler(trans_t::trans_completion_handler);


ep_t* fmiss_learn_trans_t::get_ep_entry() {
    ep_t *other_ep_entry = NULL;
    ep_l3_key_t l3_key = {0};

    l3_key.vrf_id = this->trans_key_ptr()->vrf_id;
    memcpy(&l3_key.ip_addr, &this->ip_addr_, sizeof(ip_addr_t));

    /* Find the EP entry for this IP address */
    other_ep_entry = find_ep_by_l3_key(&l3_key);

    return other_ep_entry;
}

bool fmiss_learn_trans_t::fmiss_learn_fsm_t::add_ip_entry(fsm_state_ctx ctx,
                                          fsm_event_data fsm_data)
{
    fmiss_learn_trans_t *trans = reinterpret_cast<fmiss_learn_trans_t *>(ctx);
    fmiss_learn_event_data_t *data = reinterpret_cast<fmiss_learn_event_data_t*>(fsm_data);
    hal_ret_t ret;

    ret = eplearn_ip_move_process(data->ep_handle,
            &trans->ip_entry_key_ptr()->ip_addr, FMISS_LEARN);

    if (ret != HAL_RET_OK) {
       trans->log_error("IP move process failed, skipping IP add.");
       trans->sm_->throw_event(FMISS_LEARN_ERROR, NULL);
       return false;
    }

    ep_t *ep_entry = find_ep_by_handle(data->ep_handle);
    trans->log_info("Trying to add IP to EP entry.");
    ret = endpoint_update_ip_add(ep_entry,
            &trans->ip_entry_key_ptr()->ip_addr, EP_FLAGS_LEARN_SRC_FLOW_MISS);

    if (ret != HAL_RET_OK) {
       trans->log_error("IP add update failed");
       trans->sm_->throw_event(FMISS_LEARN_ERROR, NULL);
       return false;
    }

    trans->log_info("Successfully added IP to EP entry.");
    fmiss_learn_trans_t::fmiss_learn_key_ht()->insert((void *)trans, &trans->ht_ctxt_);
    fmiss_learn_trans_t::fmiss_learn_ip_entry_ht()->insert((void *)trans,
                                                &trans->ip_entry_ht_ctxt_);


    return true;
}

bool fmiss_learn_trans_t::fmiss_learn_fsm_t::del_ip_entry(fsm_state_ctx ctx,
        fsm_event_data fsm_data)
{
    fmiss_learn_trans_t *trans = reinterpret_cast<fmiss_learn_trans_t *>(ctx);
    fmiss_learn_event_data_t *data = reinterpret_cast<fmiss_learn_event_data_t*>(fsm_data);
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

bool fmiss_learn_trans_t::fmiss_learn_fsm_t::process_fmiss_learn_request(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    fmiss_learn_trans_t *trans = reinterpret_cast<fmiss_learn_trans_t *>(ctx);
    fmiss_learn_event_data_t *data = reinterpret_cast<fmiss_learn_event_data_t*>(fsm_data);
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
        trans->sm_->throw_event(FMISS_LEARN_ERROR, NULL);
        return false;
    }


    /* Setup IP entry key for new transaction first */
    trans->set_up_ip_entry_key(ip_addr);

    fmiss_learn_trans_t *other_trans = reinterpret_cast<fmiss_learn_trans_t *>(
        fmiss_learn_trans_t::fmiss_learn_ip_entry_ht()->lookup(
                (trans->ip_entry_key_ptr())));

    if (other_trans != NULL) {
        trans->log_error("Found an existing transaction, will be removed.");
        other_trans->log_error("Initiating transaction delete.");
        fmiss_learn_trans_t::process_transaction(other_trans, FMISS_LEARN_REMOVE, NULL);
    }

    if(data->in_fte_pipeline) {
        /* Everything looks good, do a commit after FTE pipeline completed. */
        ADD_COMPLETION_HANDLER(trans, FMISS_IP_ADD,
                ep_entry->hal_handle, *ip_addr);
    } else {
        add_ip_entry(ctx, fsm_data);
    }

    return true;
}



bool fmiss_learn_trans_t::fmiss_learn_fsm_t::process_fmiss_learn_timeout(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data)
{
    fmiss_learn_trans_t *trans = reinterpret_cast<fmiss_learn_trans_t *>(ctx);

    trans->fmiss_learn_timer_ctx = nullptr;

    return true;
}

void fmiss_learn_trans_t::set_up_ip_entry_key(const ip_addr_t *ip_addr) {
    memcpy(&this->ip_addr_, ip_addr, sizeof(ip_addr_t));
    init_ip_entry_key(ip_addr, this->trans_key_ptr()->vrf_id,
            this->ip_entry_key_ptr());
}

bool fmiss_learn_trans_t::protocol_address_match(const ip_addr_t *ip_addr) {
    return (!memcmp(&this->ip_addr_, ip_addr, sizeof(ip_addr_t)));
}

void *fmiss_learn_trans_t::operator new(size_t size) {
    void *p = fmiss_learn_trans_alloc_init();
    return p;
}

void fmiss_learn_trans_t::operator delete(void *p) {
    fmiss_learn_trans_t *trans = reinterpret_cast<fmiss_learn_trans_t *>(p);
    fmiss_learn_trans_free(trans);
}

fmiss_learn_trans_t::fmiss_learn_trans_t(fmiss_learn_trans_key_t *trans_key,  fte::ctx_t &ctx) {

    memcpy(&this->trans_key_, trans_key, sizeof(fmiss_learn_trans_key_t));
    this->sm_ = new fsm_state_machine_t(get_sm_def_func, FMISS_LEARN_INIT, FMISS_LEARN_DONE,
                                        FMISS_LEARN_TIMEOUT, FMISS_LEARN_REMOVE,
                                        (fsm_state_ctx)this,
                                        get_timer_func);
    this->ht_ctxt_.reset();
    this->ip_entry_ht_ctxt_.reset();
}

void fmiss_learn_trans_t::process_event(fmiss_learn_fsm_event_t event, fsm_event_data data) {
    this->sm_->process_event(event, data);
}

fmiss_learn_trans_t::~fmiss_learn_trans_t() {
    this->log_info("Deleting transaction.");
    this->reset();
    delete this->sm_;
}

void fmiss_learn_trans_t::reset() {
    hal_ret_t ret;
    ep_t * ep_entry;

    fmiss_learn_trans_t::fmiss_learn_key_ht()->remove(&this->trans_key_);
    fmiss_learn_trans_t::fmiss_learn_ip_entry_ht()->remove(this->ip_entry_key_ptr());
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



void fmiss_learn_trans_t::init_fmiss_learn_trans_key(const uint8_t *hw_addr, const ep_t *ep,
                                        ip_addr_t *ip_addr, fmiss_learn_trans_key_t *trans_key) {
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

fmiss_learn_fsm_state_t fmiss_learn_trans_t::get_state() {
    return (fmiss_learn_fsm_state_t)this->sm_->get_state();
}

hal_ret_t
fmiss_learn_process_ip_move(hal_handle_t ep_handle, const ip_addr_t *ip_addr) {

    return fmiss_learn_trans_t::process_ip_move(ep_handle, ip_addr,
            fmiss_learn_trans_t::get_ip_ht());
}

}  // namespace eplearn
}  // namespace hal
