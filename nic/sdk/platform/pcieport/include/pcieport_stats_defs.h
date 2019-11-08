/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef PCIEPORT_STATS_DEF
#define PCIEPORT_STATS_DEF(st)
#endif

PCIEPORT_STATS_DEF(intr_total)
PCIEPORT_STATS_DEF(intr_ltssmst_early)
PCIEPORT_STATS_DEF(intr_ltssmst)
PCIEPORT_STATS_DEF(intr_linkup2dn)
PCIEPORT_STATS_DEF(intr_linkdn2up)
PCIEPORT_STATS_DEF(intr_rstup2dn)
PCIEPORT_STATS_DEF(intr_rstdn2up)
PCIEPORT_STATS_DEF(intr_secbus)

PCIEPORT_STATS_DEF(linkup)
PCIEPORT_STATS_DEF(hostup)
PCIEPORT_STATS_DEF(phypolllast)
PCIEPORT_STATS_DEF(phypollmax)
PCIEPORT_STATS_DEF(phypollperstn)
PCIEPORT_STATS_DEF(phypollfail)
PCIEPORT_STATS_DEF(gatepolllast)
PCIEPORT_STATS_DEF(gatepollmax)
PCIEPORT_STATS_DEF(markerpolllast)
PCIEPORT_STATS_DEF(markerpollmax)
PCIEPORT_STATS_DEF(axipendpolllast)
PCIEPORT_STATS_DEF(axipendpollmax)
PCIEPORT_STATS_DEF(faults)
PCIEPORT_STATS_DEF(powerdown)

#undef PCIEPORT_STATS_DEF
