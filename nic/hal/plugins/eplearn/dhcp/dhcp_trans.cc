#include "nic/include/pd_api.hpp"
#include "dhcp_trans.hpp"
#include <arpa/inet.h>
#include "dhcp_packet.hpp"
#include "nic/hal/plugins/eplearn/eplearn.hpp"
#include "nic/include/interface_api.hpp"

namespace hal {
namespace eplearn {

void *dhcptrans_get_key_func(void *entry) {
    HAL_ASSERT(entry != NULL);
    return (void *)(((dhcp_trans_t *)entry)->trans_key_ptr());
}

uint32_t dhcptrans_compute_hash_func(void *key, uint32_t ht_size) {
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(dhcp_trans_key_t)) % ht_size;
}

bool dhcptrans_compare_key_func(void *key1, void *key2) {
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (memcmp(key1, key2, sizeof(dhcp_trans_key_t)) == 0) {
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
                                sizeof(dhcp_trans_t), 16,
                                false, true, true);
ht *dhcp_trans_t::dhcplearn_key_ht_ =
    ht::factory(HAL_MAX_DHCP_TRANS, dhcptrans_get_key_func,
                dhcptrans_compute_hash_func,
                dhcptrans_compare_key_func);

ht *dhcp_trans_t::dhcplearn_ip_entry_ht_ =
    ht::factory(HAL_MAX_DHCP_TRANS,
                trans_get_ip_entry_key_func,
                trans_compute_ip_entry_hash_func,
                trans_compare_ip_entry_key_func);

/* For now setting a high time for DOL testing
 * But, all these should be configurable.
 */
#define INIT_TIMEOUT        120 * TIME_MSECS_PER_SEC
#define SELECTING_TIMEOUT   120 * TIME_MSECS_PER_SEC
#define REQUESTING_TIMEOUT  120 * TIME_MSECS_PER_SEC

#define ADD_COMPLETION_HANDLER(__trans, __event, __ep_handle, __new_ip_addr)           \
    uint32_t __trans_cnt = eplearn_info->trans_ctx_cnt;                                \
    eplearn_info->trans_ctx[__trans_cnt].trans = __trans;                              \
    eplearn_info->trans_ctx[__trans_cnt].dhcp_data.event = __event;                    \
    eplearn_info->trans_ctx[__trans_cnt].dhcp_data.ep_handle = (__ep_handle);          \
    eplearn_info->trans_ctx[__trans_cnt].dhcp_data.new_ip_addr =  (__new_ip_addr);     \
    eplearn_info->trans_ctx_cnt++;                                                     \
    fte_ctx->register_completion_handler(dhcp_completion_hdlr);


// clang-format off
void dhcp_trans_t::dhcp_fsm_t::_init_state_machine() {
#define SM_FUNC(__func) SM_BIND_NON_STATIC(dhcp_fsm_t, __func)
#define SM_FUNC_ARG_1(__func) SM_BIND_NON_STATIC_ARGS_1(dhcp_fsm_t, __func)
    FSM_SM_BEGIN((sm_def))
        FSM_STATE_BEGIN(DHCP_INIT, INIT_TIMEOUT, NULL, NULL)
            FSM_TRANSITION(DHCP_DISCOVER, SM_FUNC(process_dhcp_discover), DHCP_SELECTING)
            FSM_TRANSITION(DHCP_REQUEST, SM_FUNC(process_dhcp_request), DHCP_REQUESTING)
            FSM_TRANSITION(DHCP_INFORM, SM_FUNC(process_dhcp_inform), DHCP_BOUND)
            FSM_TRANSITION(DHCP_ERROR, NULL, DHCP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(DHCP_SELECTING, SELECTING_TIMEOUT, NULL, NULL)
            FSM_TRANSITION(DHCP_OFFER, SM_FUNC(process_dhcp_offer), DHCP_SELECTING)
            FSM_TRANSITION(DHCP_INVALID_PACKET, NULL, DHCP_DONE)
            FSM_TRANSITION(DHCP_ERROR, NULL, DHCP_DONE)
            FSM_TRANSITION(DHCP_REQUEST, SM_FUNC(process_dhcp_request), DHCP_REQUESTING)
            FSM_TRANSITION(DHCP_REBIND, SM_FUNC(process_dhcp_request), DHCP_REQUESTING)
        FSM_STATE_END
        FSM_STATE_BEGIN(DHCP_REQUESTING, REQUESTING_TIMEOUT, NULL, NULL)
            FSM_TRANSITION(DHCP_ACK, SM_FUNC(process_dhcp_ack), DHCP_BOUND)
            FSM_TRANSITION(DHCP_REQUEST, SM_FUNC(process_dhcp_request), DHCP_REQUESTING)
            FSM_TRANSITION(DHCP_REBIND, NULL, DHCP_REQUESTING)
            FSM_TRANSITION(DHCP_NACK, SM_FUNC(process_dhcp_end), DHCP_DONE)
            FSM_TRANSITION(DHCP_ERROR, SM_FUNC(process_dhcp_end), DHCP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(DHCP_RENEWING, 0, NULL, NULL)
            FSM_TRANSITION(DHCP_ACK, SM_FUNC(process_dhcp_ack), DHCP_BOUND)
            FSM_TRANSITION(DHCP_REBIND, NULL, DHCP_REBINDING)
            FSM_TRANSITION(DHCP_REQUEST, SM_FUNC(process_dhcp_request_after_bound), DHCP_RENEWING)
            FSM_TRANSITION(DHCP_LEASE_TIMEOUT, SM_FUNC(process_dhcp_bound_timeout), DHCP_DONE)
            FSM_TRANSITION(DHCP_RELEASE, SM_FUNC(process_dhcp_end), DHCP_DONE)
            FSM_TRANSITION(DHCP_NACK, SM_FUNC(process_dhcp_end), DHCP_DONE)
            FSM_TRANSITION(DHCP_ERROR, SM_FUNC(process_dhcp_end), DHCP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(DHCP_REBINDING, 0, NULL, NULL)
            FSM_TRANSITION(DHCP_ACK, SM_FUNC(process_dhcp_ack), DHCP_BOUND)
            FSM_TRANSITION(DHCP_REQUEST, SM_FUNC(process_dhcp_request), DHCP_REQUESTING)
            FSM_TRANSITION(DHCP_REBIND, NULL, DHCP_REBINDING)
            FSM_TRANSITION(DHCP_LEASE_TIMEOUT, SM_FUNC(process_dhcp_bound_timeout), DHCP_DONE)
            FSM_TRANSITION(DHCP_RELEASE, SM_FUNC(process_dhcp_end), DHCP_DONE)
            FSM_TRANSITION(DHCP_NACK, SM_FUNC(process_dhcp_end), DHCP_DONE)
            FSM_TRANSITION(DHCP_ERROR, SM_FUNC(process_dhcp_end), DHCP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(DHCP_BOUND, 0, SM_FUNC_ARG_1(bound_entry_func), NULL)
            FSM_TRANSITION(DHCP_REQUEST, SM_FUNC(process_dhcp_request_after_bound), DHCP_RENEWING)
            FSM_TRANSITION(DHCP_REBIND, NULL, DHCP_REBINDING)
            FSM_TRANSITION(DHCP_INFORM, SM_FUNC(process_dhcp_inform), DHCP_BOUND)
            FSM_TRANSITION(DHCP_IP_ADD, SM_FUNC(add_ip_entry), DHCP_BOUND)
            FSM_TRANSITION(DHCP_DECLINE, SM_FUNC(process_dhcp_end), DHCP_DONE)
            FSM_TRANSITION(DHCP_RELEASE, SM_FUNC(process_dhcp_end), DHCP_DONE)
            FSM_TRANSITION(DHCP_INVALID_PACKET, SM_FUNC(process_dhcp_end), DHCP_DONE)
            FSM_TRANSITION(DHCP_LEASE_TIMEOUT, SM_FUNC(process_dhcp_bound_timeout), DHCP_DONE)
        FSM_STATE_END
        FSM_STATE_BEGIN(DHCP_DONE, 0, NULL, NULL) //No done timeout, it will be deleted.
        FSM_STATE_END
    FSM_SM_END
    this->set_state_machine(sm_def);
}
// clang-format on


static void lease_timeout_handler(void *timer, uint32_t timer_id, void *ctxt) {
    fsm_state_machine_t* sm_ = reinterpret_cast<fsm_state_machine_t*>(ctxt);
    sm_->reset_timer();
    trans_t* trans =
        reinterpret_cast<trans_t*>(sm_->get_ctx());
    trans_t::process_transaction(trans, DHCP_LEASE_TIMEOUT, NULL);
}

static void dhcp_completion_hdlr (fte::ctx_t& ctx, bool status) {
    eplearn_info_t *eplearn_info = (eplearn_info_t*)\
                ctx.feature_state(FTE_FEATURE_EP_LEARN);

    dhcp_event_data event_data = { 0 };

    for (uint32_t i = 0; i < eplearn_info->trans_ctx_cnt; i++) {
        event_data.fte_ctx = &ctx;
        event_data.ep_handle =
                eplearn_info->trans_ctx[i].dhcp_data.ep_handle;
        event_data.in_fte_pipeline = false;
        event_data.decoded_packet =
                eplearn_info->trans_ctx[i].dhcp_data.decoded_packet;
        event_data.new_ip_addr = eplearn_info->trans_ctx[i].dhcp_data.new_ip_addr;

        eplearn_info->trans_ctx[i].trans->log_info("Executing completion handler ");
        dhcp_trans_t::process_transaction(eplearn_info->trans_ctx[i].trans,
                eplearn_info->trans_ctx[i].dhcp_data.event,
                (fsm_event_data)(&event_data));
    }
}

static hal_ret_t
update_flow_fwding(fte::ctx_t *fte_ctx)
{
    fte::flow_update_t flowupd;
    const fte::cpu_rxhdr_t* cpu_rxhdr_ = fte_ctx->cpu_rxhdr();
    hal::l2seg_t *dl2seg;
    ether_header_t * ethhdr;
    ep_t *dep = nullptr;
    if_t *dif;
    hal_ret_t ret;
    hal::pd::pd_find_l2seg_by_hwid_args_t args;

    dep = fte_ctx->dep();

    if (dep == nullptr) {
#if 0
        dl2seg =  hal::pd::find_l2seg_by_hwid(cpu_rxhdr_->lkp_vrf);
#endif
        args.hwid = cpu_rxhdr_->lkp_vrf;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_FIND_L2SEG_BY_HWID, (void *)&args);
        dl2seg = args.l2seg;

        HAL_ASSERT(dl2seg != nullptr);
        ethhdr = (ether_header_t *)(fte_ctx->pkt() + cpu_rxhdr_->l2_offset);
        dep = hal::find_ep_by_l2_key(dl2seg->seg_id, ethhdr->dmac);
        if (dep == nullptr) {
            HAL_TRACE_ERR("Destination endpoint not found.");
            ret = HAL_RET_EP_NOT_FOUND;
            goto out;
        }
    }

    dif = hal::find_if_by_handle(dep->if_handle);
    bzero(&flowupd, sizeof(flowupd));
    flowupd.fwding.dif = dif;
    if (dif && dif->if_type == intf::IF_TYPE_ENIC) {
        hal::lif_t *lif = if_get_lif(dif);
        if (lif == NULL) {
            ret = HAL_RET_LIF_NOT_FOUND;
            goto out;
        }
        flowupd.type =  fte::FLOWUPD_FWDING_INFO;
        flowupd.fwding.qtype = lif_get_qtype(lif, intf::LIF_QUEUE_PURPOSE_RX);
        flowupd.fwding.qid_en = 1;
        flowupd.fwding.qid = 0;
        pd::pd_if_get_lport_id_args_t args;
        args.pi_if = flowupd.fwding.dif;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_IF_GET_LPORT_ID, (void *)&args);
        flowupd.fwding.lport = args.lport_id;
        // flowupd.fwding.lport = hal::pd::if_get_lport_id(flowupd.fwding.dif);
        flowupd.fwding.dl2seg = dl2seg;
        ret = fte_ctx->update_flow(flowupd);
        if (ret != HAL_RET_OK) {
            goto out;
        }
        flowupd.type =  fte::FLOWUPD_ACTION;
        flowupd.action = session::FLOW_ACTION_ALLOW;
        ret = fte_ctx->update_flow(flowupd);
    } else {
        ret = HAL_RET_IF_NOT_FOUND;
    }

out:
     return ret;
}

bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_discover(fsm_state_ctx ctx,
                                                     fsm_event_data fsm_data) {
    //dhcp_event_data *data = reinterpret_cast<dhcp_event_data*>(fsm_data);
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);


    dhcp_trans->log_info("Processing dhcp discover.");
    dhcp_trans_t *existing_trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&dhcp_trans->trans_key_));

    if (existing_trans == nullptr) {
        dhcp_trans_t::dhcplearn_key_ht()->insert((void *)dhcp_trans,
                                                &dhcp_trans->ht_ctxt_);
    }

    return true;
}

bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_inform(fsm_state_ctx ctx,
                                                    fsm_event_data fsm_data) {
    dhcp_event_data *data = reinterpret_cast<dhcp_event_data*>(fsm_data);
    const struct packet *decoded_packet = data->decoded_packet;
    fte::ctx_t *fte_ctx = data->fte_ctx;
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);
    struct dhcp_packet *raw = decoded_packet->raw;
    dhcp_ctx *dhcp_ctx = &dhcp_trans->ctx_;
    struct option_data option_data;
    ep_t *ep_entry = fte_ctx->sep();
    eplearn_info_t *eplearn_info = (eplearn_info_t*)\
                fte_ctx->feature_state(FTE_FEATURE_EP_LEARN);
    ip_addr_t ip_addr = {0};
    hal_ret_t ret;
    bool rc = true;
    dhcp_trans_t *existing_trans;

    ip_addr.addr.v4_addr = ntohl(raw->yiaddr.s_addr);

    ret = dhcp_lookup_option(decoded_packet,
                             DHO_DHCP_SERVER_IDENTIFIER, &option_data);
    if (ret != HAL_RET_OK) {
        dhcp_trans->sm_->throw_event(DHCP_INVALID_PACKET, NULL);
        rc = false;
        goto out;
    }

    if (ep_entry == nullptr) {
        HAL_TRACE_ERR("DHCP client Endpoint entry not found.");
        dhcp_trans->sm_->throw_event(DHCP_INVALID_PACKET, NULL);
        rc = false;
        goto out;
    }

    memcpy(&(dhcp_ctx->server_identifer_), option_data.data,
           sizeof(dhcp_ctx->server_identifer_));

    existing_trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&dhcp_trans->trans_key_));

    if (existing_trans == nullptr) {
        dhcp_trans_t::dhcplearn_key_ht()->insert((void *)dhcp_trans,
                                                &dhcp_trans->ht_ctxt_);
    }

    if (!memcmp(&(dhcp_trans->ip_entry_key_ptr()->ip_addr),
            &ip_addr, sizeof(ip_addr)) == 0) {
        /*
         * Modify only if the IP address is different from the previous one.
         */
        if (data->in_fte_pipeline) {
            ADD_COMPLETION_HANDLER(dhcp_trans, DHCP_IP_ADD,
                    ep_entry->hal_handle, ip_addr);
        } else {
            rc = add_ip_entry(ctx, fsm_data);
            if (!rc) {
                goto out;
            }
        }
    }

out:
    return rc;
}


bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_end(fsm_state_ctx ctx,
                                                fsm_event_data fsm_data) {
    dhcp_event_data *data = reinterpret_cast<dhcp_event_data*>(fsm_data);
    fte::ctx_t *fte_ctx = data->fte_ctx;
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);
    ep_t *ep_entry = fte_ctx->sep();
    eplearn_info_t *eplearn_info = (eplearn_info_t*)\
                fte_ctx->feature_state(FTE_FEATURE_EP_LEARN);
    ip_addr_t ip_addr = {0};
    bool rc = true;

    if (data->in_fte_pipeline) {
        ADD_COMPLETION_HANDLER(dhcp_trans, data->event,
                ep_entry->hal_handle, ip_addr);
        rc = false;
        /* Don't change state, done soon after pipeline completed */
        goto out;
    }

out:
    return rc;
}

bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_request(fsm_state_ctx ctx,
                                                    fsm_event_data fsm_data) {
    dhcp_event_data *data = reinterpret_cast<dhcp_event_data*>(fsm_data);
    const struct packet *decoded_packet = data->decoded_packet;
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);
    dhcp_ctx *dhcp_ctx = &dhcp_trans->ctx_;
    struct option_data option_data;
    hal_ret_t ret;

    dhcp_trans->log_info("Processing dhcp request...");
    ret = dhcp_lookup_option(decoded_packet,
                            DHO_DHCP_SERVER_IDENTIFIER, &option_data);
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
    dhcp_event_data *data = reinterpret_cast<dhcp_event_data*>(fsm_data);
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);
    fte::ctx_t *fte_ctx = data->fte_ctx;
    bool rc = true;

    if (is_broadcast(*fte_ctx)) {
        dhcp_trans->sm_->throw_event(DHCP_REBIND, fsm_data);
        rc = false;
        goto out;
    }

out:
    return rc;
}

void dhcp_trans_t::reset() {
    hal_ret_t ret;
    ep_t * ep_entry;

    dhcp_trans_t::dhcplearn_key_ht()->remove(&this->trans_key_);
    dhcp_trans_t::dhcplearn_ip_entry_ht()->remove(this->ip_entry_key_ptr());

    this->stop_lease_timer();
    ep_entry = this->get_ep_entry();
    if (ep_entry != NULL) {
        ret = endpoint_update_ip_delete(ep_entry,
                &this->ip_entry_key_ptr()->ip_addr, EP_FLAGS_LEARN_SRC_DHCP);
        if (ret != HAL_RET_OK) {
            this->log_error("IP delete update failed");
        }
    }

    this->sm_->stop_state_timer();
}

bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_offer(fsm_state_ctx ctx,
                                                fsm_event_data fsm_data) {
    dhcp_event_data *data = reinterpret_cast<dhcp_event_data*>(fsm_data);
    fte::ctx_t *fte_ctx = data->fte_ctx;
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);
    hal_ret_t ret;

    dhcp_trans->log_info("Processing DHCP OFfer.");
    /*
     * Update flow forwarding information.
     */
    ret = update_flow_fwding(fte_ctx);

    if (ret != HAL_RET_OK) {
        dhcp_trans->log_error("Unable to update flow fwding info, dropping transaction");
        dhcp_trans->sm_->throw_event(DHCP_ERROR, NULL);
        return false;
    }


    return true;
}

bool dhcp_trans_t::dhcp_fsm_t::add_ip_entry(fsm_state_ctx ctx,
                                          fsm_event_data fsm_data)
{
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);
    dhcp_event_data *data = reinterpret_cast<dhcp_event_data*>(fsm_data);
    ep_t *ep_entry = find_ep_by_handle(data->ep_handle);
    hal_ret_t ret;
    bool rc = true;
    ip_addr_t zero_ip = { 0 };


    if ((memcmp(&(dhcp_trans->ip_entry_key_ptr()->ip_addr),
            &zero_ip, sizeof(ip_addr_t))) &&
        (memcmp(&(dhcp_trans->ip_entry_key_ptr()->ip_addr),
                   (&data->new_ip_addr), sizeof(ip_addr_t)))) {
        /*
         * Restart Transaction if IP address is different.
         * */
        restart_transaction(ctx, fsm_data);
        /* Do a lookup again to make sure we have updated EP reference. */
        ep_entry = find_ep_by_handle(data->ep_handle);
    }

    init_ip_entry_key((&data->new_ip_addr),
                      dhcp_trans->trans_key_ptr()->vrf_id,
                      dhcp_trans->ip_entry_key_ptr());

    ret = endpoint_update_ip_add(ep_entry,
            &dhcp_trans->ip_entry_key_ptr()->ip_addr,
            EP_FLAGS_LEARN_SRC_DHCP);

    if (ret != HAL_RET_OK) {
        dhcp_trans->log_error("IP add update failed");
        dhcp_trans->sm_->throw_event(DHCP_ERROR, NULL);
       /* We should probably drop this packet as this DHCP request may not be valid one */
        rc = false;
    } else {
        dhcp_trans->log_info("IP add update successful");
    }

    dhcp_trans_t::dhcplearn_ip_entry_ht()->insert(
        (void *)dhcp_trans, &dhcp_trans->ip_entry_ht_ctxt_);

    dhcp_trans->start_lease_timer();

    return rc;
}

void dhcp_trans_t::start_lease_timer() {

    if (this->lease_timer_ctx) {
        dhcp_timer_->delete_timer(this->lease_timer_ctx);
    }
    if (this->ctx_.renewal_time_) {
        this->lease_timer_ctx  = dhcp_timer_->add_timer_with_custom_handler(
                this->ctx_.renewal_time_ * TIME_MSECS_PER_SEC,
                this->sm_, lease_timeout_handler);
    } else {
        this->lease_timer_ctx = nullptr;
    }
}

void dhcp_trans_t::stop_lease_timer() {

    if (this->lease_timer_ctx) {
        dhcp_timer_->delete_timer(this->lease_timer_ctx);
    }
    this->lease_timer_ctx = nullptr;
}

bool dhcp_trans_t::dhcp_fsm_t::restart_transaction(fsm_state_ctx ctx,
                                          fsm_event_data fsm_data)
{
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);

    dhcp_trans->log_info("Restarting DHCP Transaction.");
    dhcp_trans->reset();
    dhcp_trans_t::dhcplearn_key_ht()->insert((void *)dhcp_trans,
                                            &dhcp_trans->ht_ctxt_);

    return true;
}


bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_ack(fsm_state_ctx ctx,
                                                fsm_event_data fsm_data) {
    dhcp_event_data *data = reinterpret_cast<dhcp_event_data*>(fsm_data);
    const struct packet *decoded_packet = data->decoded_packet;
    fte::ctx_t *fte_ctx = data->fte_ctx;
    struct dhcp_packet *raw = decoded_packet->raw;
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);
    dhcp_ctx *dhcp_ctx = &dhcp_trans->ctx_;
    eplearn_info_t *eplearn_info = (eplearn_info_t*)\
                fte_ctx->feature_state(FTE_FEATURE_EP_LEARN);
    struct option_data option_data;
    hal_ret_t ret;
    ep_t *ep_entry;
    ip_addr_t ip_addr = { 0 };
    bool rc = true;

    ip_addr.addr.v4_addr = ntohl(raw->yiaddr.s_addr);

    dhcp_trans->log_info("Processing dhcp ack.");
    HAL_TRACE_INFO("Received IP {:#x}, {}", ip_addr, ipaddr2str(&ip_addr));
    ep_entry = fte_ctx->sep();
    if (ep_entry == nullptr) {
        dhcp_trans->log_error("DHCP Server Endpoint entry not found.");
        dhcp_trans->sm_->throw_event(DHCP_INVALID_PACKET, NULL);
        return false;
    }

    /* This is DHCP ack, find the corresponding EP by l2 key */
    ep_entry = find_ep_by_l2_key(ep_entry->l2_key.l2_segid,
            dhcp_trans->trans_key_ptr()->mac_addr);
    if (ep_entry == nullptr) {
        dhcp_trans->log_error("DHCP client endpoint entry not found.");
        dhcp_trans->sm_->throw_event(DHCP_ERROR, NULL);
        return false;
    }

    dhcp_ctx->yiaddr_ = raw->yiaddr;
    ret = dhcp_lookup_option(decoded_packet, DHO_DHCP_RENEWAL_TIME, &option_data);
    if (ret != HAL_RET_OK) {
        dhcp_trans->log_error("Invalid DHCP packet, renewal time not found.");
        dhcp_trans->sm_->throw_event(DHCP_INVALID_PACKET, NULL);
        return false;
    }
    memcpy(&(dhcp_ctx->renewal_time_), option_data.data,
           sizeof(dhcp_ctx->renewal_time_));

    ret = dhcp_lookup_option(decoded_packet, DHO_DHCP_REBINDING_TIME, &option_data);
    if (ret != HAL_RET_OK) {
        dhcp_trans->log_error("Invalid DHCP packet, rebinding time not found.");
        dhcp_trans->sm_->throw_event(DHCP_INVALID_PACKET, NULL);
        return false;
    }
    memcpy(&(dhcp_ctx->rebinding_time_), option_data.data,
           sizeof(dhcp_ctx->rebinding_time_));

    ret = dhcp_lookup_option(decoded_packet, DHO_DHCP_LEASE_TIME, &option_data);
    if (ret != HAL_RET_OK) {
        dhcp_trans->log_error("Invalid DHCP packet, lease time not found.");
        dhcp_trans->sm_->throw_event(DHCP_INVALID_PACKET, NULL);
        return false;
    }
    memcpy(&(dhcp_ctx->lease_time_), option_data.data,
           sizeof(dhcp_ctx->lease_time_));

    ret = dhcp_lookup_option(decoded_packet, DHO_ROUTERS, &option_data);
    if (ret == HAL_RET_OK) {
        memcpy(&(dhcp_ctx->default_gateway_), option_data.data,
                sizeof(dhcp_ctx->default_gateway_));
    }

    if (memcmp(&(dhcp_trans->ip_entry_key_ptr()->ip_addr),
            &ip_addr, sizeof(ip_addr)) == 0) {
        dhcp_trans->start_lease_timer();
    } else {
        if (data->in_fte_pipeline) {
            ADD_COMPLETION_HANDLER(dhcp_trans, DHCP_IP_ADD,
                    ep_entry->hal_handle, ip_addr);
        } else {
            rc = add_ip_entry(ctx, fsm_data);
            if (!rc) {
                goto out;
            }
        }
    }

    /*
     * Finally update flow forwarding information.
     */
    ret = update_flow_fwding(fte_ctx);

    if (ret != HAL_RET_OK) {
        dhcp_trans->log_error("Unable to update flow fwding info, dropping transaction");
        dhcp_trans->sm_->throw_event(DHCP_ERROR, NULL);
        rc = false;
        goto out;
    }

 out:
    return rc;
}

bool dhcp_trans_t::dhcp_fsm_t::process_dhcp_bound_timeout(fsm_state_ctx ctx,
                                                          fsm_event_data data) {
    dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);

    dhcp_trans->log_info("Bound timed out.");
    return true;
}

void dhcp_trans_t::dhcp_fsm_t::bound_entry_func(fsm_state_ctx ctx) {
    //dhcp_trans_t *dhcp_trans = reinterpret_cast<dhcp_trans_t *>(ctx);
    //dhcp_trans->sm_->set_current_state_timeout(dhcp_ctx->lease_time_);
    //TODO : ADD Bound timer here.
}

dhcp_trans_t::dhcp_trans_t(struct packet *dhcp_packet, fte::ctx_t &ctx) {
    this->ctx_.init(dhcp_packet);
    this->sm_ = new fsm_state_machine_t(get_sm_def_func, DHCP_INIT, DHCP_DONE,
                                      DHCP_TIMEOUT, (fsm_state_ctx)this,
                                      get_timer_func);
    init_dhcp_trans_key(dhcp_packet->raw->chaddr, this->ctx_.xid_,
                        ctx.sep(), &this->trans_key_);
}

void dhcp_trans_t::init_dhcp_trans_key(const uint8_t *chaddr, uint32_t xid,
                                       const ep_t *ep,
                                       dhcp_trans_key_t *trans_key_) {
    for (uint32_t i = 0; i < ETHER_ADDR_LEN; ++i) {
        trans_key_->mac_addr[i] = chaddr[i];
    }
    trans_key_->xid = xid;
    vrf_t *vrf = vrf_lookup_by_handle(ep->vrf_handle);
    if (vrf == NULL) {
        HAL_ABORT(0);
    }
    trans_key_->vrf_id = vrf->vrf_id;
}

void dhcp_trans_t::process_event(dhcp_fsm_event_t event, fsm_event_data data) {
    this->sm_->process_event(event, data);
}

dhcp_trans_t::~dhcp_trans_t() {
    this->reset();
    delete this->sm_;
}

}  // namespace eplearn
}  // namespace hal
