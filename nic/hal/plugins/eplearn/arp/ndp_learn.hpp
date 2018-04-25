//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef HAL_PLUGINS_EPLEARN_ARP_NDP_LEARN_HPP_
#define HAL_PLUGINS_EPLEARN_ARP_NDP_LEARN_HPP_

#include "nic/include/base.h"
#include "nic/include/fte_ctx.hpp"

namespace hal {
namespace eplearn {

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t reserved;
    ipv6_addr_t target_addr;
    uint8_t options[];
}icmpv6_neigh_solict_t;

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
#if __BYTE_ORDER == __BIG_ENDIAN
    uint32_t r : 1;
    uint32_t s : 1;
    uint32_t o : 1;
    uint32_t reserved1 : 5;
    uint32_t reserved2 : 24;
   #else
    uint32_t reserved1 : 5;
    uint32_t o : 1;
    uint32_t s : 1;
    uint32_t r : 1;
    uint32_t reserved2 : 24;
   #endif
    ipv6_addr_t targetAddr;
    uint8_t options[];
}icmpv6_neigh_advert_t;

typedef struct {
    uint8_t type;
    uint8_t length;
    uint8_t value[];
} icmpv6_option_info_t;

typedef enum {
   NDP_OPT_SOURCE_LINK_LAYER_ADDR = 1,
   NDP_OPT_TARGET_LINK_LAYER_ADDR = 2,
   NDP_OPT_PREFIX_INFORMATION     = 3,
} icmpv6_option_t;

bool is_neighbor_discovery_flow(const hal::flow_key_t *key);
hal_ret_t neighbor_disc_process_packet(fte::ctx_t &ctx);

}  // namespace eplearn
}  // namespace hal


#endif /* HAL_PLUGINS_EPLEARN_ARP_NDP_LEARN_HPP_ */
