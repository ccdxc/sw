//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef HAL_PLUGINS_EPLEARN_EPLEARN_HPP_
#define HAL_PLUGINS_EPLEARN_EPLEARN_HPP_
#include "common/trans.hpp"
#include "flow_miss/fmiss_learn_trans.hpp"
#include "nic/hal/plugins/eplearn/dhcp/dhcp_trans.hpp"
#include "nic/hal/plugins/eplearn/arp/arp_trans.hpp"

namespace hal {
namespace eplearn {


enum ep_learn_type_t {
    ARP_LEARN,
    DHCP_LEARN,
    FMISS_LEARN,
    EP_LEARN_MAX,
};

#define EPLEARN_TRANS_CTX_MAX 2

const std::string FTE_FEATURE_EP_LEARN("pensando.io/eplearn:eplearn");

typedef struct eplearn_trans_ctx_s {
    trans_t       *trans; /* Transaction that has to be processed */
    uint32_t       event;
    union {
        dhcp_event_data  dhcp_data;
        arp_event_data_t arp_data;
        fmiss_learn_event_data_t fmiss_learn_data;
    }event_data;
} eplearn_trans_ctx_t;

typedef struct eplearn_info_s {
    eplearn_trans_ctx_t trans_ctx[EPLEARN_TRANS_CTX_MAX];
    uint32_t            trans_ctx_cnt;
} eplearn_info_t;

static inline bool
is_broadcast(fte::ctx_t &ctx) {
    const fte::cpu_rxhdr_t* cpu_hdr = ctx.cpu_rxhdr();
    ether_header_t *eth_hdr = (ether_header_t*)(ctx.pkt() + cpu_hdr->l2_offset);
    for (int i = 0; i < ETHER_ADDR_LEN; i++) {
        if (eth_hdr->dmac[i] != 0xff) {
            return false;
        }
    }
    return true;
}

/* Callback type registered by each ep learning type to check whether to delete learn't IP */
typedef hal_ret_t (*ip_move_check_handler)(hal_handle_t ep_handle, const ip_addr_t *ip_addr);

void register_ip_move_check_handler(ip_move_check_handler handler, ep_learn_type_t type);

hal_ret_t
eplearn_ip_move_process(hal_handle_t ep_handle, const ip_addr_t *ip_addr,
        ep_learn_type_t type);


}
}


#endif /* HAL_PLUGINS_EPLEARN_EPLEARN_HPP_ */
