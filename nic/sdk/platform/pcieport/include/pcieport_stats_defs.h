/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef PCIEPORT_STATS_DEF
#define PCIEPORT_STATS_DEF(st)
#endif

PCIEPORT_STATS_DEF(intr_total)
PCIEPORT_STATS_DEF(intr_polled)
PCIEPORT_STATS_DEF(intr_perstn)
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

#define int_mac_stats link_dn2up_int
PCIEPORT_STATS_DEF(link_dn2up_int)
PCIEPORT_STATS_DEF(link_up2dn_int)
PCIEPORT_STATS_DEF(sec_bus_rst_int)
PCIEPORT_STATS_DEF(rst_up2dn_int)
PCIEPORT_STATS_DEF(rst_dn2up_int)
PCIEPORT_STATS_DEF(portgate_open2close_int)
PCIEPORT_STATS_DEF(ltssm_st_changed_int)
PCIEPORT_STATS_DEF(sec_busnum_changed_int)
PCIEPORT_STATS_DEF(rc_pme_int)
PCIEPORT_STATS_DEF(rc_aerr_int)
PCIEPORT_STATS_DEF(rc_serr_int)
PCIEPORT_STATS_DEF(rc_hpe_int)
PCIEPORT_STATS_DEF(rc_eq_req_int)
PCIEPORT_STATS_DEF(rc_dpc_int)
PCIEPORT_STATS_DEF(pm_turnoff_int)
PCIEPORT_STATS_DEF(txbfr_overflow_int)
PCIEPORT_STATS_DEF(rxtlp_err_int)
PCIEPORT_STATS_DEF(tl_flr_req_int)
PCIEPORT_STATS_DEF(rc_legacy_intpin_changed_int)

#undef PCIEPORT_STATS_DEF
