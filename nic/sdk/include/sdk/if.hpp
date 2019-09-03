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

enum {
    IF_TYPE_NONE    = 0,
    IF_TYPE_ETH     = 1,
    IF_TYPE_ETH_PC  = 2,
    IF_TYPE_TUNNEL  = 3,
    IF_TYPE_MGMT    = 4,
};

#define IF_TYPE_SHIFT                            28
#define IF_SLOT_SHIFT                            24
#define IF_PARENT_PORT_SHIFT                     16
#define IF_TYPE_MASK                             0xF
#define IF_SLOT_MASK                             0xF
#define IF_PARENT_PORT_MASK                      0xFF
#define IF_CHILD_PORT_MASK                       0xFF
#define IF_DEFAULT_CHILD_PORT                    1

///< interface index is formed from
///<     t_ --> type of the interface (4 bits)
///<     s_ --> physical slot of the card  (4 bits)
///<     p_ --> parent port (1 based) (8 bits)
///<     c_ --> child port (1 based) (8 bits)
#define IFINDEX(t_, s_, p_, c_)                      \
            ((t_ << IF_TYPE_SHIFT)        |          \
             ((s_) << IF_SLOT_SHIFT)      |          \
             (p_ << IF_PARENT_PORT_SHIFT) | (c_))

#define ETH_IFINDEX(s_, p_, c_)    IFINDEX(IF_TYPE_ETH, (s_), (p_), (c_))

#define IFINDEX_TO_IFTYPE(ifindex_)         \
            ((ifindex_ >> IF_TYPE_SHIFT) & IF_TYPE_MASK)

#define IFINDEX_TO_PARENT_PORT(ifindex_)    \
            ((ifindex >> IF_PARENT_PORT_SHIFT) & IF_PARENT_PORT_MASK)

#define IFINDEX_TO_CHILD_PORT(ifindex_)    \
            (ifindex & IF_CHILD_PORT_MASK)

#endif    // __IF_HPP__
