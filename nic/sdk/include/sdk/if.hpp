//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
///----------------------------------------------------------------------------
///
/// \file
/// common interface APIs & macros
///
///----------------------------------------------------------------------------

#ifndef __IF_HPP__
#define __IF_HPP__

#include <string>

enum {
    IF_TYPE_NONE       = 0,
    IF_TYPE_ETH        = 1,
    IF_TYPE_ETH_PC     = 2,
    IF_TYPE_TUNNEL     = 3,
    IF_TYPE_MGMT       = 4,
    IF_TYPE_UPLINK     = 5,
    IF_TYPE_UPLINK_PC  = 6,
    IF_TYPE_L3         = 7,
    IF_TYPE_LIF        = 8,
    IF_TYPE_LOOPBACK   = 9,
    IF_TYPE_CONTROL    = 10,
    IF_TYPE_HOST       = 11,
};

#define IFINDEX_INVALID                          0x0

#define IF_TYPE_SHIFT                            28
#define IF_TYPE_MASK                             0xF
#define ETH_IF_SLOT_SHIFT                        24
#define ETH_IF_PARENT_PORT_SHIFT                 16
#define ETH_IF_SLOT_MASK                         0xF
#define ETH_IF_PARENT_PORT_MASK                  0xFF
#define ETH_IF_CHILD_PORT_MASK                   0xFFFF
#define ETH_IF_DEFAULT_CHILD_PORT                1
#define ETH_IF_DEFAULT_SLOT                      1
#define LIF_IF_LIF_ID_MASK                       0xFFFFFF
#define LOOPBACK_IF_ID_MASK                      0xFFFFFF

///< interface index is formed from
///<     t_ --> type of the interface (4 bits)
///<     s_ --> physical slot of the card  (4 bits)
///<     p_ --> parent port (1 based) (8 bits)
///<     c_ --> child port (0 based) (16 bits)
///< based on the type of the interface, the slot bits will be
///< used for a running identifier of that type of interface
///< e.g. IF_TYPE_UPLINK or IF_TYPE_TUNNEL etc.
///< NOTE: child port 0 ==> non-broken port
#define IFINDEX(t_, s_, p_, c_)                          \
            ((t_ << IF_TYPE_SHIFT)        |              \
             ((s_) << ETH_IF_SLOT_SHIFT)      |          \
             (p_ << ETH_IF_PARENT_PORT_SHIFT) | (c_))

#define ETH_IFINDEX(s_, p_, c_)    IFINDEX(IF_TYPE_ETH, (s_), (p_), (c_))
#define UPLINK_PC_IFINDEX(pc_id_)  ((IF_TYPE_UPLINK_PC << IF_TYPE_SHIFT) | \
                                    (pc_id_))
#define L3_IFINDEX(if_id_)         ((IF_TYPE_L3 << IF_TYPE_SHIFT) | (if_id_))
#define LIF_IFINDEX(if_id_)        ((uint32_t)(IF_TYPE_LIF << IF_TYPE_SHIFT) | (if_id_))
#define LOOPBACK_IFINDEX(if_id_)   ((IF_TYPE_LOOPBACK << IF_TYPE_SHIFT) | (if_id_))
#define CONTROL_IFINDEX(if_id_)    ((IF_TYPE_CONTROL << IF_TYPE_SHIFT) | (if_id_))
#define HOST_IFINDEX(if_id_)       ((IF_TYPE_HOST << IF_TYPE_SHIFT) | (if_id_))

#define IFINDEX_TO_IFTYPE(ifindex_)         \
            ((ifindex_ >> IF_TYPE_SHIFT) & IF_TYPE_MASK)

#define ETH_IFINDEX_TO_SLOT(ifindex_)           \
            ((ifindex_ >> ETH_IF_SLOT_SHIFT) & ETH_IF_SLOT_MASK)

#define ETH_IFINDEX_TO_PARENT_PORT(ifindex_)    \
            ((ifindex_ >> ETH_IF_PARENT_PORT_SHIFT) & ETH_IF_PARENT_PORT_MASK)

#define ETH_IFINDEX_TO_CHILD_PORT(ifindex_)    \
            (ifindex_ & ETH_IF_CHILD_PORT_MASK)

#define ETH_IFINDEX_TO_UPLINK_IFINDEX(ifindex_)      \
            ((IF_TYPE_UPLINK << IF_TYPE_SHIFT) |     \
             ((ifindex_) & ~(IF_TYPE_MASK << IF_TYPE_SHIFT)))

#define UPLINK_IFINDEX_TO_ETH_IFINDEX(ifindex_)      \
            ((IF_TYPE_ETH << IF_TYPE_SHIFT) |        \
             ((ifindex_) & ~(IF_TYPE_MASK << IF_TYPE_SHIFT)))

#define LIF_IFINDEX_TO_LIF_ID(ifindex_)    \
            (ifindex_ & LIF_IF_LIF_ID_MASK)

#define LOOPBACK_IFINDEX_TO_LOOPBACK_IF_ID(ifindex_)    \
            (ifindex_ & LOOPBACK_IF_ID_MASK)

static inline std::string
ifindex_to_type_str (uint32_t ifindex) {
    uint32_t type = IFINDEX_TO_IFTYPE(ifindex);

	switch (type) {
	case IF_TYPE_ETH:
		return "Eth";
	case IF_TYPE_ETH_PC:
		return "EthPC";
	case IF_TYPE_TUNNEL:
		return "Tunnel";
	case IF_TYPE_MGMT:
		return "Mgmt";
	case IF_TYPE_UPLINK:
		return "Uplink";
	case IF_TYPE_UPLINK_PC:
		return "UplinkPC";
	case IF_TYPE_L3:
		return "L3";
	case IF_TYPE_LIF:
		return "Lif";
    case IF_TYPE_LOOPBACK:
        return "Loopback";
	case IF_TYPE_CONTROL:
		return "Control";
    case IF_TYPE_HOST:
        return "Host";
    case IF_TYPE_NONE:
    default:
        return "None";
	}
}

static inline std::string
eth_ifindex_to_str (uint32_t ifindex)
{
    uint32_t slot;
    uint32_t parent_port;

    if (ifindex != IFINDEX_INVALID) {
        slot = ETH_IFINDEX_TO_SLOT(ifindex);
        parent_port = ETH_IFINDEX_TO_PARENT_PORT(ifindex);
        return ifindex_to_type_str(ifindex) + std::to_string(slot) + "/"
                                            + std::to_string(parent_port);
    }
    return "-";
}

#endif    // __IF_HPP__
