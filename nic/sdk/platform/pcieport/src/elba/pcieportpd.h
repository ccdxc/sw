/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#ifndef __PCIEPORTPD_H__
#define __PCIEPORTPD_H__

#include "platform/pcieport/include/elba/pcieportpd.h"

#define ASIC_(REG)              ELB_ ##REG

union pcieport_u;
typedef union pcieport_u pcieport_t;
struct pciemgr_params_s;
typedef struct pciemgr_params_s pciemgr_params_t;

int pcieportpd_validate_hostconfig(pcieport_t *p);
int pcieportpd_hostconfig(pcieport_t *p, const pciemgr_params_t *params);
int pcieportpd_config_host(pcieport_t *p);
int pcieportpd_config_rc(pcieport_t *p);
int pcieportpd_config_powerdown(pcieport_t *p);
int pcieportpd_serdes_init(void);
int pcieportpd_serdes_reset(void);
void pcieportpd_mac_set_ids(pcieport_t *p);
void pcieportpd_select_pcie_refclk(const int port, const int host_clock);
int pcieportpd_is_accessible(const int port);

void pcieportpd_intr_inherit(pcieport_t *p);
int pcieportpd_check_for_intr(const int port);
void pcieportpd_intr_enable(const int port);
void pcieportpd_intr_disable(const int port);
int pcieportpd_intr_init(const int port);
int pcieportpd_poll_init(const int port);

#endif /* __PCIEPORTPD_H__ */
