#include "dhcp_trans.hpp"
#include <arpa/inet.h>
#include "dhcp_packet.hpp"

namespace hal {
namespace network {

void *dhcptrans_get_key_func(void *entry) {
    HAL_ASSERT(entry != NULL);
    return (void *)(((dhcp_trans_t *)entry)->trans_key_ptr());
}

uint32_t dhcptrans_compute_hash_func(void *key, uint32_t ht_size) {
    return utils::hash_algo::fnv_hash(key, sizeof(dhcp_trans_key_t)) % ht_size;
}

bool dhcptrans_compare_key_func(void *key1, void *key2) {
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (memcmp(key1, key2, sizeof(dhcp_trans_key_t)) == 0) {
        return true;
    }
    return false;
}

void *dhcptrans_get_ip_entry_key_func(void *entry) {
    HAL_ASSERT(entry != NULL);
    return (void *)(((dhcp_trans_t *)entry)->ip_entry_key_ptr());
}

uint32_t dhcptrans_compute_ip_entry_hash_func(void *key, uint32_t ht_size) {
    return utils::hash_algo::fnv_hash(key, sizeof(dhcp_ip_entry_key_t)) %
           ht_size;
}

bool dhcptrans_compare_ip_entry_key_func(void *key1, void *key2) {
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (memcmp(key1, key2, sizeof(dhcp_ip_entry_key_t)) == 0) {
        return true;
    }
    return false;
}


void dhcp_ctx::init(struct packet* dhcp_packet) {
	struct dhcp_packet* raw = dhcp_packet->raw;
	this->xid_ = ntohl(raw->xid);
	memcpy(this->chaddr_, raw->chaddr, 16);
	this->server_identifer_ = 0;
	this->renewal_time_ = 0;
	this->rebinding_time_ = 0;
	this->lease_time_ = 0;
	bzero(&this->yiaddr_, sizeof(struct in_addr));
	bzero(&this->subnet_mask_, sizeof(struct in_addr));
}

/* Make sure only one instance of State machine is present and all transactions
 * should use the same state machine */
dhcp_trans_t::dhcp_fsm_t *dhcp_trans_t::dhcp_fsm_ = new dhcp_fsm_t();
dhcp_trans_t::trans_timer_t *dhcp_trans_t::dhcp_timer_ =
            new trans_timer_t(DHCP_TIMER_ID);
slab *dhcp_trans_t::dhcplearn_slab_ = slab::factory("dhcpLearn", HAL_SLAB_DHCP_LEARN,
                                sizeof(hal::network::dhcp_trans_t), 16,
                                false, true, true, true);
ht *dhcp_trans_t::dhcplearn_key_ht_ =
    ht::factory(HAL_MAX_DHCP_TRANS, hal::network::dhcptrans_get_key_func,
                hal::network::dhcptrans_compute_hash_func,
                hal::network::dhcptrans_compare_key_func);

ht *dhcp_trans_t::dhcplearn_ip_entry_ht_ =
    ht::factory(HAL_MAX_DHCP_TRANS,
                hal::network::dhcptrans_get_ip_entry_key_func,
                hal::network::dhcptrans_compute_ip_entry_hash_func,
                hal::network::dhcptrans_compare_ip_entry_key_func);

#define INIT_TIMEOUT        120 * TIME_MSECS_PER_SEC
#define SELECTING_TIMEOUT   120 * TIME_MSECS_PER_SEC
#define REQUESTING_TIMEOUT  120 * TIME_MSECS_PER_SEC
// This is default, entry func can override this.
#define BOUND_TIMEOUT       120 * TIME_MSECS_PER_SEC
#define RENEWING_TIMEOUT    120 * TIME_MSECS_PER_SEC

// clang-format off
void dhcp_trans_t::dhcp_fsm_t::_init_state_machine() {
#define SM_FUNC(__func) SM_BIND_NON_STATIC(dhcp_fsm_t, __func)
#define SM_FUNC_ARG_1(__func) SM_BIND_NON_STATIC_ARGS_1(dhcp_fsm_t, __func)
    FSM_SM_BEGIN((sm_def))
        FSM_STATE_BEGIN(DHCP_INIT, INIT_TIMEOUT, NULL, NULL)
            FSM_TRANSITION(DHCP_DISCOVER, SM_FUNC(process_dhcp_discover), DHCP_SELECTING)
            FSM_TRANSITION(DHCP_REQUEST, SM_FUNC(process_dhcp_request), DHCP_REQUESTING)
            FSM_TRANSITION(DHCP_INFORM, SM_FUNC(process_dhcp_request), DHCP_BOUND)
        FSM_STATE_END
        FSM_STATE_BEGIN(DHCP_SELECTING, SELECTING_TIMEOUT, NULL, NULL)
            FSM_TRANSITION(DHCP_OFFER, NULL, DHCP_SELECTING)
            FSM_TRANSITION(DHCP_INVALID_PACKET, NULL, DHCP_DONE)
            FSM_TRANSITION(DHCP_ERROR, NULL, DHCP_DONE)
            FSM_TRANSITION(DHCP_REQUEST, SM_FUNC(process_dhcp_request), DHCP_REQUESTING)
            FSM_TRANSITION(DHCP_NACK, SM_FUNC(process_dhcp_request), DHCP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(DHCP_REQUESTING, REQUESTING_TIMEOUT, NULL, NULL)
            FSM_TRANSITION(DHCP_ACK, SM_FUNC(process_dhcp_ack), DHCP_BOUND)
            FSM_TRANSITION(DHCP_REQUEST, SM_FUNC(process_dhcp_request), DHCP_REQUESTING)
            FSM_TRANSITION(DHCP_NACK, NULL, DHCP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(DHCP_BOUND, BOUND_TIMEOUT, SM_FUNC_ARG_1(bound_entry_func), NULL)
            FSM_TRANSITION(DHCP_REQUEST, SM_FUNC(process_dhcp_request_after_bound), DHCP_RENEWING)
            FSM_TRANSITION(DHCP_INFORM, SM_FUNC(process_dhcp_request_after_bound), DHCP_BOUND)
            FSM_TRANSITION(DHCP_DECLINE, NULL, DHCP_DONE)
            FSM_TRANSITION(DHCP_RELEASE, SM_FUNC(process_dhcp_release), DHCP_DONE)
            FSM_TRANSITION(DHCP_TIMEOUT, SM_FUNC(process_dhcp_bound_timeout), DHCP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(DHCP_RENEWING, RENEWING_TIMEOUT, NULL, NULL)
            FSM_TRANSITION(DHCP_ACK, SM_FUNC(process_dhcp_ack), DHCP_BOUND)
            FSM_TRANSITION(DHCP_REQUEST, SM_FUNC(process_dhcp_request), DHCP_REQUESTING)
            FSM_TRANSITION(DHCP_NACK, NULL, DHCP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(DHCP_DONE, 0, NULL, NULL) //No done timeout, it will be deleted.
        FSM_STATE_END
    FSM_SM_END
    this->set_state_machine(sm_def);
}
// clang-format on
bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_discover(fsm_state_ctx ctx,
                                                     fsm_event_data fsm_data) {
    //dhcp_event_data *data = reinterpret_cast<dhcp_event_data*>(fsm_data);
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);

    dhcp_trans_t *existing_trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&dhcp_trans->trans_key_));

    if (existing_trans == nullptr) {
        dhcp_trans_t::dhcplearn_key_ht()->insert((void *)dhcp_trans,
                                                &dhcp_trans->ht_ctxt_);
    }

    return true;
}

bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_request(fsm_state_ctx ctx,
                                                    fsm_event_data fsm_data) {
    dhcp_event_data *data = reinterpret_cast<dhcp_event_data*>(fsm_data);
    const struct packet *decoded_packet = data->decoded_packet;
    //const fte::ctx_t *fte_ctx = data->fte_ctx;
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);
    dhcp_ctx *dhcp_ctx = &dhcp_trans->ctx_;
    struct option_data option_data;
    hal_ret_t ret;

    ret =
        dhcp_lookup_option(decoded_packet, DHO_DHCP_SERVER_IDENTIFIER, &option_data);
    if (ret != HAL_RET_OK) {
        dhcp_trans->sm_->throw_event(DHCP_INVALID_PACKET, NULL);
        return false;
    }

    memcpy(&(dhcp_ctx->server_identifer_), option_data.data,
           sizeof(dhcp_ctx->server_identifer_));

    dhcp_trans_t *existing_trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&dhcp_trans->trans_key_));

    if (existing_trans == nullptr) {
        dhcp_trans_t::dhcplearn_key_ht()->insert((void *)dhcp_trans,
                                                &dhcp_trans->ht_ctxt_);
    }

    return true;
}

bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_request_after_bound(
    fsm_state_ctx ctx, fsm_event_data fsm_data) {
    //dhcp_event_data *data = reinterpret_cast<dhcp_event_data*>(fsm_data);
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);
    /*
     * TODO: Add HAL EP Delete IP API Here.
     */

    /* Remove this IP reference as new IP will be retrieved. */
    dhcp_trans_t::dhcplearn_ip_entry_ht()->remove(&dhcp_trans->ip_entry_key_);
    return this->process_dhcp_request(ctx, fsm_data);
}

bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_release(fsm_state_ctx ctx,
                                                    fsm_event_data data) {
    /*
     * TODO: Add HAL EP Delete IP API Here.
     */
    return true;
}

bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_ack(fsm_state_ctx ctx,
                                                fsm_event_data fsm_data) {
    dhcp_event_data *data = reinterpret_cast<dhcp_event_data*>(fsm_data);
    const struct packet *decoded_packet = data->decoded_packet;
    const fte::ctx_t *fte_ctx = data->fte_ctx;
    struct dhcp_packet *raw = decoded_packet->raw;
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);
    dhcp_ctx *dhcp_ctx = &dhcp_trans->ctx_;
    struct option_data option_data;
    hal_ret_t ret;
    vrf_t *vrf;

    ep_t *ep_entry = fte_ctx->sep();

    if (ep_entry == nullptr) {
        HAL_TRACE_ERR("Endpoint entry not found.");
        dhcp_trans->sm_->throw_event(DHCP_INVALID_PACKET, NULL);
        return false;
    }

    dhcp_ctx->yiaddr_ = raw->yiaddr;
    ret = dhcp_lookup_option(decoded_packet, DHO_DHCP_RENEWAL_TIME, &option_data);
    if (ret != HAL_RET_OK) {
        dhcp_trans->sm_->throw_event(DHCP_INVALID_PACKET, NULL);
        return false;
    }
    memcpy(&(dhcp_ctx->renewal_time_), option_data.data,
           sizeof(dhcp_ctx->renewal_time_));

    ret = dhcp_lookup_option(decoded_packet, DHO_DHCP_REBINDING_TIME, &option_data);
    if (ret != HAL_RET_OK) {
        dhcp_trans->sm_->throw_event(DHCP_INVALID_PACKET, NULL);
        return false;
    }
    memcpy(&(dhcp_ctx->rebinding_time_), option_data.data,
           sizeof(dhcp_ctx->rebinding_time_));

    ret = dhcp_lookup_option(decoded_packet, DHO_DHCP_LEASE_TIME, &option_data);
    if (ret != HAL_RET_OK) {
        dhcp_trans->sm_->throw_event(DHCP_INVALID_PACKET, NULL);
        return false;
    }
    memcpy(&(dhcp_ctx->lease_time_), option_data.data,
           sizeof(dhcp_ctx->lease_time_));

    /*
     * TODO: Add HAL EP update API Here.
     */
    vrf = vrf_lookup_by_handle(ep_entry->vrf_handle);
    if (vrf == NULL) {
       HAL_TRACE_ERR("pi-ep:{}:unable to find vrf", __FUNCTION__);
       dhcp_trans->sm_->throw_event(DHCP_ERROR, NULL);
       return false;
    }

    init_dhcp_ip_entry_key((uint8_t *)(&raw->yiaddr.s_addr), vrf->vrf_id,
                           &dhcp_trans->ip_entry_key_);
    dhcp_trans_t::dhcplearn_ip_entry_ht()->insert(
        (void *)dhcp_trans, &dhcp_trans->ip_entry_ht_ctxt_);
    return true;
}

bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_bound_timeout(fsm_state_ctx ctx,
                                                          fsm_event_data data) {
    // DhcpTransaction *dhcp_trans = reinterpret_cast<DhcpTransaction *>(ctx);
    /*
     * TODO: Add HAL EP delete API Here.
     */

    // dhcp_trans_t::dhcplearn_ip_entry_ht()->remove(&dhcp_trans->ip_entry_key_);
    return true;
}

void dhcp_trans_t::dhcp_fsm_t::bound_entry_func(fsm_state_ctx ctx) {
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);
    dhcp_ctx *dhcp_ctx = &dhcp_trans->ctx_;
    dhcp_trans->sm_->set_current_state_timeout(dhcp_ctx->lease_time_);
}

dhcp_trans_t::dhcp_trans_t(struct packet *dhcp_packet, fte::ctx_t &ctx) {
    this->ctx_.init(dhcp_packet);
    this->sm_ = new fsm_state_machine_t(get_sm_def_func, DHCP_INIT, DHCP_DONE,
                                      DHCP_TIMEOUT, (fsm_state_ctx)this,
                                      get_timer_func);
    init_dhcp_trans_key(dhcp_packet->raw->chaddr, this->ctx_.xid_,
                        &this->trans_key_);
}

void dhcp_trans_t::init_dhcp_trans_key(const uint8_t *chaddr, uint32_t xid,
                                       dhcp_trans_key_t *trans_key_) {
    for (uint32_t i = 0; i < ETHER_ADDR_LEN; ++i) {
        trans_key_->mac_addr[i] = chaddr[i];
    }
    trans_key_->xid = xid;
}

void dhcp_trans_t::init_dhcp_ip_entry_key(const uint8_t *protocol_address,
                                          vrf_id_t vrf_id,
                                          dhcp_ip_entry_key_t *ip_entry_key_) {
    *ip_entry_key_ = {0};
    memcpy(&ip_entry_key_->ip_addr.addr.v4_addr, protocol_address,
           sizeof(ip_entry_key_->ip_addr.addr.v4_addr));
    ip_entry_key_->vrf_id = vrf_id;
}

void dhcp_trans_t::process_event(dhcp_fsm_event_t event, fsm_event_data data) {
    this->sm_->process_event(event, data);
}

dhcp_trans_t::~dhcp_trans_t() {
    dhcp_trans_t::dhcplearn_key_ht()->remove(&this->trans_key_);
    dhcp_trans_t::dhcplearn_ip_entry_ht()->remove(&this->ip_entry_key_);
    this->sm_->stop_state_timer();
    delete this->sm_;
}

}  // namespace network
}  // namespace hal
