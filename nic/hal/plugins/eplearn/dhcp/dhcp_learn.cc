/*
 * dhcp.cc
 *
 *  Created on: Aug 9, 2017
 *      Author: sudhiaithal
 */

#include <netinet/in.h>
// clang-format off
#include "dhcp_trans.hpp"
#include "dhcp_packet.hpp"
// clang-format on

// using namespace DHCP_EP_LEARN;

namespace hal {
namespace network {



void dhcp_init()
{
    dhcp_lib_init();
}

static hal_ret_t dhcp_process_request_internal(struct packet *decoded_packet,
                                               fte::ctx_t &ctx,
                                               dhcp_fsm_event_t event,
                                               bool add_entry) {
    struct dhcp_packet *raw_pkt = decoded_packet->raw;
    uint32_t xid = ntohl(raw_pkt->xid);
    dhcp_trans_t *trans;
    dhcp_trans_key_t trans_key;
    dhcp_event_data event_data;

    dhcp_trans_t::init_dhcp_trans_key(raw_pkt->chaddr, xid,
            ctx.sep(), &trans_key);
    trans = dhcp_trans_t::find_dhcptrans_by_id(trans_key);
    if (trans == NULL) {
        if (add_entry) {
            trans = new dhcp_trans_t(decoded_packet, ctx);
        } else {
            return HAL_RET_ENTRY_NOT_FOUND;
        }
    }

    event_data.decoded_packet = decoded_packet;
    event_data.fte_ctx = &ctx;
    dhcp_trans_t::process_transaction(trans, event,
                                         (fsm_event_data)(&event_data));
    return HAL_RET_OK;
}

bool is_dhcp_flow(const hal::flow_key_t *key) {
    return (key->sport == 68 || key->dport == 68);
}

typedef std::function<void(struct packet *decoded_packet, fte::ctx_t &)>
    process_func;

process_func proc_function_handler(dhcp_fsm_event_t event, bool add_on_entry) {
    return [=](struct packet *pkt, fte::ctx_t &ctx) {
        dhcp_process_request_internal(pkt, ctx, event, add_on_entry);
    };
}

static std::map<int, process_func> process_func_map = {
    {DHCPDISCOVER, proc_function_handler(DHCP_DISCOVER, true)},
    {DHCPOFFER, proc_function_handler(DHCP_OFFER, false)},
    {DHCPREQUEST, proc_function_handler(DHCP_REQUEST, true)},
    {DHCPINFORM, proc_function_handler(DHCP_INFORM, true)},
    {DHCPRELEASE, proc_function_handler(DHCP_RELEASE, true)},
    {DHCPACK, proc_function_handler(DHCP_ACK, false)},
    {DHCPNAK, proc_function_handler(DHCP_NACK, false)},
    {DHCPDECLINE, proc_function_handler(DHCP_DECLINE, true)},
};

hal_ret_t dhcp_process_packet(fte::ctx_t &ctx) {
    struct packet *decoded_packet = NULL;
    const uint8_t *buf = ctx.pkt();
    uint32_t len = ctx.pkt_len();

    hal_ret_t ret = parse_dhcp_packet(buf, len, &decoded_packet);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error parsing DHCP packet");
        return ret;
    }

    struct option_data data;
    ret = dhcp_lookup_option(decoded_packet, 53, &data);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    auto it = process_func_map.find(data.data[0]);
    if (it != process_func_map.end()) {
        it->second(decoded_packet, ctx);
    }

    if (decoded_packet != NULL) {
        free_dhcp_packet(&decoded_packet);
    }

    return ret;
}

}  // namespace network
}  // namespace hal
