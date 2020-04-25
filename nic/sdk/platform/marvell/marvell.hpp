/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#ifndef __MARVELL_H__
#define __MARVELL_H__

#include <stdint.h>
#include "lib/catalog/catalog.hpp"

// -------------------------------------------
// Marvell switch port connections on Naples25
// -------------------------------------------
// Port: Desc                    Interface
// -------------------------------------------
// P0:   Capri                   SGMII
// P1:   SGMII to MTP (Hardware) SGMII
// P2:   No connect
// P3:   RJ-45 (Naples mgmt)     PHY
// P4:   No connect
// P5:   ALOM (BMC/iLO)          RMII
// P6:   No connect

#define MARVELL_NPORTS      7       // number of ports in the marvell switch

#define MARVELL_PORT_UPDOWN_SHIFT   11          // Single bit
#define MARVELL_PORT_UPDOWN_MASK    0x1

#define MARVELL_PORT_DUPLEX_SHIFT   10          // Single bit
#define MARVELL_PORT_DUPLEX_MASK    0x1

#define MARVELL_PORT_SPEED_SHIFT    8           // Two bit data
#define MARVELL_PORT_SPEED_MASK     0x3

#define MARVELL_PORT_TXPAUSE_SHIFT  5           // Single bit
#define MARVELL_PORT_TXPAUSE_MASK   0x1

#define MARVELL_PORT_FCTRL_SHIFT    4           // Single bit
#define MARVELL_PORT_FCTRL_MASK     0x1

namespace sdk {
namespace marvell {

typedef struct marvell_cfg_s_ {
    sdk::lib::catalog *catalog;
} marvell_cfg_t;
extern marvell_cfg_t g_marvell_cfg;

typedef struct marvell_port_stats_ {
    uint64_t in_good_octets;
    uint64_t out_octets;
    uint32_t in_bad_octets;
    uint32_t in_unicast;
    uint32_t in_broadcast;
    uint32_t in_multicast;
    uint32_t in_pause;
    uint32_t in_undersize;
    uint32_t in_fragments;
    uint32_t in_oversize;
    uint32_t in_jabber;
    uint32_t in_rx_err;
    uint32_t in_fcs_err;
    uint32_t out_unicast;
    uint32_t out_broadcast;
    uint32_t out_multicast;
    uint32_t out_fcs_err;
    uint32_t out_pause;
    uint32_t out_collisions;
    uint32_t out_deferred;
    uint32_t out_single;
    uint32_t out_multiple;
    uint32_t out_excessive;
    uint32_t out_late;
} marvell_port_stats_t;

static inline const char *
marvell_get_descr (uint8_t port)
{
    switch (port) {
        case 0:
            return "Connected to Asic";
            break;
        case 1:
            return "SGMII to MTP";
            break;
        case 3:
            return "Out of band Management";
            break;
        case 5:
            return "BMC";
            break;
        // fall through
    }
    return "Unused";
}

static inline void
marvell_get_status_updown (uint16_t status, bool *up)
{
    *up = (status >> MARVELL_PORT_UPDOWN_SHIFT) & MARVELL_PORT_UPDOWN_MASK;
}

static inline void
marvell_get_status_duplex (uint16_t status, bool *fullduplex)
{
    *fullduplex = (status >> MARVELL_PORT_DUPLEX_SHIFT) &
                  MARVELL_PORT_DUPLEX_MASK;
}

static inline void
marvell_get_status_speed (uint16_t status, uint8_t *speed)
{
    *speed = (status >> MARVELL_PORT_SPEED_SHIFT) & MARVELL_PORT_SPEED_MASK;
}

static inline void
marvell_get_status_txpause (uint16_t status, bool *txpause)
{
    *txpause = (status >> MARVELL_PORT_TXPAUSE_SHIFT) &
               MARVELL_PORT_TXPAUSE_MASK;
}

static inline void
marvell_get_status_flowctrl (uint16_t status, bool *fctrl)
{
    *fctrl = (status >> MARVELL_PORT_FCTRL_SHIFT) & MARVELL_PORT_FCTRL_MASK;
}

// extern functions
void marvell_switch_init(marvell_cfg_t *marvell_cfg);
int marvell_get_port_status(uint8_t port, uint16_t *data);
int marvell_get_port_stats(uint8_t port, marvell_port_stats_t *stats);

}   // namespace marvell
}   // namespace sdk

using sdk::marvell::marvell_cfg_t;

#endif  /* __MARVELL_H__ */
