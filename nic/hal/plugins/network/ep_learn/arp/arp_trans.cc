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

namespace network {

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

void *arptrans_get_ip_entry_key_func(void *entry) {
    HAL_ASSERT(entry != NULL);
    return (void *)(((arp_trans_t *)entry)->ip_entry_key_ptr());
}

uint32_t arptrans_compute_ip_entry_hash_func(void *key, uint32_t ht_size) {
    return utils::hash_algo::fnv_hash(key,
                                      sizeof(arp_ip_entry_key_t)) % ht_size;
}

bool arptrans_compare_ip_entry_key_func(void *key1, void *key2) {
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (memcmp(key1, key2, sizeof(arp_ip_entry_key_t)) == 0) {
        return true;
    }
    return false;
}

/* Make sure only one instance of State machine is present and all transactions
 * should use the same state machine */
arp_trans_t::arp_fsm_t *arp_trans_t::arp_fsm_ = new arp_fsm_t();
arp_trans_t::trans_timer_t *arp_trans_t::arp_timer_ =
        new trans_timer_t(ARP_TIMER_ID);

#define INIT_TIMEOUT       120 * TIME_MSECS_PER_SEC
#define SELECTING_TIMEOUT  120 * TIME_MSECS_PER_SEC
#define REQUESTING_TIMEOUT 120 * TIME_MSECS_PER_SEC
// This is default, entry func can override this.
#define BOUND_TIMEOUT      20  * TIME_MSECS_PER_MIN
#define RENEWING_TIMEOUT   120 * TIME_MSECS_PER_SEC

// clang-format off
void arp_trans_t::arp_fsm_t::_init_state_machine() {
#define SM_FUNC(__func) SM_BIND_NON_STATIC(arp_fsm_t, __func)
#define SM_FUNC_ARG_1(__func) SM_BIND_NON_STATIC_ARGS_1(arp_fsm_t, __func)
    FSM_SM_BEGIN((sm_def))
        FSM_STATE_BEGIN(ARP_INIT, INIT_TIMEOUT, NULL, NULL)
            FSM_TRANSITION(ARP_ADD, SM_FUNC(process_arp_request), ARP_BOUND)
            FSM_TRANSITION(ARP_ERROR, NULL, ARP_DONE)
            FSM_TRANSITION(ARP_DUPLICATE, NULL, ARP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(ARP_BOUND, BOUND_TIMEOUT, NULL, NULL)
            FSM_TRANSITION(ARP_TIMEOUT, NULL, ARP_DONE)
            FSM_TRANSITION(ARP_REMOVE, NULL, ARP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(ARP_DONE, 0, NULL, NULL)
        FSM_STATE_END
    FSM_SM_END
    this->set_state_machine(sm_def);
}
// clang-format on
bool arp_trans_t::arp_fsm_t::process_arp_request(fsm_state_ctx ctx,
                                               fsm_event_data fsm_data) {
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(ctx);
    ep_l3_key_t l3_key;
    tenant_t    *tenant;
    arp_event_data_t *data = reinterpret_cast<arp_event_data_t*>(fsm_data);
    const fte::ctx_t *fte_ctx = data->fte_ctx;

    ep_t *ep_entry = fte_ctx->sep();

    if (ep_entry == nullptr) {
        HAL_TRACE_ERR("Endpoint entry not found.");
        trans->sm_->throw_event(ARP_ERROR, NULL);
        return false;
    }

    tenant = tenant_lookup_by_handle(ep_entry->tenant_handle);
    if (tenant == NULL) {
       HAL_TRACE_ERR("pi-ep:{}:unable to find tenant", __FUNCTION__);
       trans->sm_->throw_event(ARP_ERROR, NULL);
       return false;
    }

    l3_key.tenant_id = tenant->tenant_id;
    memcpy(&l3_key.ip_addr.addr.v4_addr, trans->protocol_addr_,
           sizeof(ipv4_addr_t));

    /* Find the EP entry for this IP address */
    ep_t *other_ep_entry = reinterpret_cast<ep_t *>(
        g_hal_state->ep_l3_entry_ht()->lookup(&l3_key));

    if (other_ep_entry == ep_entry) {
        // Same IP being updated for the EP entry.
        trans->sm_->throw_event(ARP_DUPLICATE, NULL);
        return false;
    }

    tenant = tenant_lookup_by_handle(ep_entry->tenant_handle);
    if (tenant == NULL) {
       HAL_TRACE_ERR("pi-ep:{}:unable to find tenant", __FUNCTION__);
       trans->sm_->throw_event(ARP_ERROR, NULL);
       return false;
    }

    /* Check if there is an existing entry already */
    arp_ip_entry_key_t ip_key = {0};
    init_arp_ip_entry_key(trans->protocol_addr_, tenant->tenant_id, &ip_key);

    arp_trans_t *other_trans = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_ip_entry_ht()->lookup(&ip_key));

    if (other_trans != NULL) {
        arp_trans_t::process_transaction(other_trans, ARP_REMOVE, NULL);
    }

    /* TODO: 1.Remove from the other ep entry this particular IP.
     * 2. Add a new entry for this EP.
     *  */

    g_hal_state->arplearn_key_ht()->insert((void *)trans, &trans->ht_ctxt_);
    g_hal_state->arplearn_ip_entry_ht()->insert((void *)trans,
                                                &trans->ip_entry_ht_ctxt_);

    return true;
}

void *arp_trans_t::operator new(size_t size) {
    void *p = arp_trans_alloc_init();

    return p;
}

void arp_trans_t::operator delete(void *p) {
    arp_trans_t *trans = reinterpret_cast<arp_trans_t *>(p);
    arp_trans_free(trans);
}

arp_trans_t::arp_trans_t(uint8_t *hw_address, uint8_t *protocol_address,
                         fte::ctx_t &ctx) {
    memcpy(this->hw_addr_, hw_address, sizeof(this->hw_addr_));
    memcpy(this->protocol_addr_, protocol_address, sizeof(this->protocol_addr_));
    init_arp_trans_key(hw_address, ctx.sep(), &this->trans_key_);
    init_arp_ip_entry_key(protocol_address, ctx.tenant()->tenant_id, &this->ip_entry_key_);
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
    g_hal_state->arplearn_key_ht()->remove(&this->trans_key_);
    g_hal_state->arplearn_ip_entry_ht()->remove(&this->ip_entry_key_);
    /*
     * TODO : Also remove the IP entry from the associated EP as well.
     */
    delete this->sm_;
    this->sm_->stop_state_timer();
}

void arp_trans_t::init_arp_trans_key(const uint8_t *hw_addr, const ep_t *ep,
                                     arp_trans_key_t *trans_key) {
    for (uint32_t i = 0; i < ETHER_ADDR_LEN; ++i) {
        trans_key->mac_addr[i] = (uint8_t)hw_addr[i];
    }
    trans_key->l2_segid = ep->l2_key.l2_segid;
}

void arp_trans_t::init_arp_ip_entry_key(const uint8_t *protocol_address,
                                        tenant_id_t tenant_id,
                                        arp_ip_entry_key_t *ip_entry_key) {
    *ip_entry_key = {0};
    memcpy(&ip_entry_key->ip_addr.addr.v4_addr, protocol_address,
           sizeof(ip_entry_key->ip_addr.addr.v4_addr));
    ip_entry_key->tenant_id = tenant_id;
}

arp_fsm_state_t arp_trans_t::get_state() {
    return (arp_fsm_state_t)this->sm_->get_state();
}

}  // namespace network
}  // namespace hal
