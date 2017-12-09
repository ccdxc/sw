#include <arpa/inet.h>
#include "nic/include/base.h"
#include "arp_trans.hpp"
#include "nic/utils/fsm/fsm.hpp"

using hal::utils::fsm_transition_t;
using hal::utils::fsm_transition_func;
using hal::utils::fsm_state_func;
using hal::utils::fsm_state_machine_def_t;
using hal::utils::fsm_state_machine_t;

namespace hal {

namespace eplearn {

void *arptrans_get_key_func(void *entry) {
    HAL_ASSERT(entry != NULL);
    return (void *)(((arp_trans_t *)entry)->trans_key_ptr());
}

uint32_t arptrans_compute_hash_func(void *key, uint32_t ht_size) {
    return utils::hash_algo::fnv_hash(key, sizeof(arp_trans_key_t)) % ht_size;
}

bool arptrans_compare_key_func(void *key1, void *key2) {
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (memcmp(key1, key2, sizeof(arp_trans_key_t)) == 0) {
        return true;
    }
    return false;
}


/* Make sure only one instance of State machine is present and all transactions
 * should use the same state machine */
arp_trans_t::arp_fsm_t *arp_trans_t::arp_fsm_ = new arp_fsm_t();
arp_trans_t::trans_timer_t *arp_trans_t::arp_timer_ =
        new trans_timer_t(ARP_TIMER_ID);
slab *arp_trans_t::arplearn_slab_ =
    slab::factory("arpLearn", HAL_SLAB_ARP_LEARN,
                  sizeof(arp_trans_t), 16, false,
                  true, true, true);
ht *arp_trans_t::arplearn_key_ht_ =
    ht::factory(HAL_MAX_ARP_TRANS, arptrans_get_key_func,
                arptrans_compute_hash_func,
                arptrans_compare_key_func);
ht *arp_trans_t::arplearn_ip_entry_ht_ =
    ht::factory(HAL_MAX_ARP_TRANS,
                trans_get_ip_entry_key_func,
                trans_compute_ip_entry_hash_func,
                trans_compare_ip_entry_key_func);

#define INIT_TIMEOUT       120 * TIME_MSECS_PER_SEC
// This is default, entry func can override this.
#define BOUND_TIMEOUT      2000  * TIME_MSECS_PER_MIN
#define RARP_ENTRY_TIMEOUT 1  * TIME_MSECS_PER_MIN

// clang-format off
void arp_trans_t::arp_fsm_t::_init_state_machine() {
#define SM_FUNC(__func) SM_BIND_NON_STATIC(arp_fsm_t, __func)
#define SM_FUNC_ARG_1(__func) SM_BIND_NON_STATIC_ARGS_1(arp_fsm_t, __func)
    FSM_SM_BEGIN((sm_def))
        FSM_STATE_BEGIN(ARP_INIT, INIT_TIMEOUT, NULL, NULL)
            FSM_TRANSITION(ARP_ADD, SM_FUNC(process_arp_request), ARP_BOUND)
            FSM_TRANSITION(RARP_REQ, SM_FUNC(process_rarp_request), RARP_INIT)
            FSM_TRANSITION(ARP_ERROR, NULL, ARP_DONE)
            FSM_TRANSITION(ARP_DUPLICATE, NULL, ARP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(RARP_INIT, RARP_ENTRY_TIMEOUT, NULL, NULL)
            FSM_TRANSITION(RARP_REPLY, SM_FUNC(process_rarp_reply),
                           ARP_BOUND)
            FSM_TRANSITION(ARP_TIMEOUT, NULL, ARP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(ARP_BOUND, BOUND_TIMEOUT, NULL, NULL)
            FSM_TRANSITION(ARP_ADD, SM_FUNC(process_arp_renewal_request),
                           ARP_BOUND)
            FSM_TRANSITION(ARP_TIMEOUT, NULL, ARP_DONE)
            FSM_TRANSITION(ARP_REMOVE, NULL, ARP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(ARP_DONE, 0, NULL, NULL)
        FSM_STATE_END
    FSM_SM_END
    this->set_state_machine(sm_def);
}
// clang-format on
ep_t* arp_trans_t::get_ep_entry() {
    ep_t *other_ep_entry = NULL;
    ep_l3_key_t l3_key = {0};

    l3_key.vrf_id = this->trans_key_ptr()->vrf_id;
    memcpy(&l3_key.ip_addr, &this->ip_addr_, sizeof(ip_addr_t));

    /* Find the EP entry for this IP address */
    other_ep_entry = find_ep_by_l3_key(&l3_key);

    return other_ep_entry;
}

bool arp_trans_t::arp_fsm_t::process_arp_request(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(ctx);
    arp_event_data_t *data = reinterpret_cast<arp_event_data_t*>(fsm_data);
    const fte::ctx_t *fte_ctx = data->fte_ctx;
    const ip_addr_t *ip_addr = data->ip_addr;
    hal_ret_t ret;
    ep_t *ep_entry = fte_ctx->sep();

    trans->log_info("Processing ARP request.");
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

    ret = endpoint_update_ip_add(ep_entry,
            &trans->ip_entry_key_ptr()->ip_addr, EP_FLAGS_LEARN_SRC_ARP);

    if (ret != HAL_RET_OK) {
       trans->log_error("IP add update failed");
       trans->sm_->throw_event(ARP_ERROR, NULL);
       /* We should probably drop this packet as this ARP request may not be valid one */
       return false;
    }

    arp_trans_t::arplearn_key_ht()->insert((void *)trans, &trans->ht_ctxt_);
    arp_trans_t::arplearn_ip_entry_ht()->insert((void *)trans,
                                                &trans->ip_entry_ht_ctxt_);

    return true;
}

bool arp_trans_t::arp_fsm_t::process_rarp_reply(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(ctx);
    arp_event_data_t *data = reinterpret_cast<arp_event_data_t*>(fsm_data);
    const fte::ctx_t *fte_ctx = data->fte_ctx;
    const ip_addr_t *ip_addr = data->ip_addr;
    hal_ret_t ret;
    ep_t *ep_entry = fte_ctx->dep();

    trans->log_info("Processing RARP reply.");
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

    ret = endpoint_update_ip_add(ep_entry,
            &trans->ip_entry_key_ptr()->ip_addr, EP_FLAGS_LEARN_SRC_ARP);

    if (ret != HAL_RET_OK) {
       trans->log_error("IP add update failed");
       trans->sm_->throw_event(ARP_ERROR, NULL);
       /* We should probably drop this packet as this ARP request may not be valid one */
       return false;
    }

    arp_trans_t::arplearn_key_ht()->insert((void *)trans, &trans->ht_ctxt_);
    arp_trans_t::arplearn_ip_entry_ht()->insert((void *)trans,
                                                &trans->ip_entry_ht_ctxt_);

    return true;
}

bool arp_trans_t::arp_fsm_t::process_rarp_request(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(ctx);
    arp_event_data_t *data = reinterpret_cast<arp_event_data_t*>(fsm_data);
    const fte::ctx_t *fte_ctx = data->fte_ctx;
    ep_t *ep_entry = fte_ctx->sep();

    trans->log_info("Processing RARP request.");
    if (ep_entry == nullptr) {
        trans->log_error("Endpoint entry not found.");
        trans->sm_->throw_event(ARP_ERROR, NULL);
        return false;
    }

    arp_trans_t::arplearn_key_ht()->insert((void *)trans, &trans->ht_ctxt_);

    return true;
}

bool arp_trans_t::arp_fsm_t::process_arp_renewal_request(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(ctx);
    arp_event_data_t *data = reinterpret_cast<arp_event_data_t*>(fsm_data);
    const fte::ctx_t *fte_ctx = data->fte_ctx;
    const ip_addr_t *ip_addr = data->ip_addr;
    ep_t *ep_entry = fte_ctx->sep();

    trans->log_info("Processing RARP renewal request.");
    if (ep_entry == nullptr) {
        trans->log_error("Endpoint entry not found.");
        trans->sm_->throw_event(ARP_ERROR, NULL);
        return false;
    }

    if (trans->protocol_address_match(ip_addr)) {
        /* Just a renewal, nothing to do, timeout will be updated. */
        trans->log_info("Protocol address same, nothing to do.");
        return true;
    }

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

arp_trans_t::arp_trans_t(const uint8_t *hw_address,
        arp_trans_type_t type, fte::ctx_t &ctx) {
    memcpy(this->hw_addr_, hw_address, sizeof(this->hw_addr_));
    init_arp_trans_key(hw_address, ctx.sep(), type, &this->trans_key_);
    this->sm_ = new fsm_state_machine_t(get_sm_def_func, ARP_INIT, ARP_DONE,
                                        ARP_TIMEOUT, (fsm_state_ctx)this,
                                        get_timer_func);
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
        ep_entry = this->get_ep_entry();
        HAL_ASSERT(ep_entry == NULL);
    }

    this->sm_->stop_state_timer();
}

void arp_trans_t::init_arp_trans_key(const uint8_t *hw_addr, const ep_t *ep,
                                     arp_trans_type_t type, arp_trans_key_t *trans_key) {
    for (uint32_t i = 0; i < ETHER_ADDR_LEN; ++i) {
        trans_key->mac_addr[i] = (uint8_t)hw_addr[i];
    }
    trans_key->l2_segid = ep->l2_key.l2_segid;
    vrf_t *vrf = vrf_lookup_by_handle(ep->vrf_handle);
    if (vrf == NULL) {
        HAL_ABORT(0);
    }
    trans_key->vrf_id = vrf->vrf_id;
    trans_key->type = type;
}

arp_fsm_state_t arp_trans_t::get_state() {
    return (arp_fsm_state_t)this->sm_->get_state();
}

}  // namespace eplearn
}  // namespace hal
