/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#ifndef __PCIEUTILPD_H__
#define __PCIEUTILPD_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include "elb_wa_c_hdr.h"

#define ASIC_ELBA
#include "platform/pciemgr/include/pciemgr.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "platform/pcieport/include/pcieport.h"
#include "platform/pcieport/include/elba/pcieportpd.h"
#include "platform/pciemgr/include/pciehw.h"
#include "platform/pciemgr/include/pciehw_dev.h"

#define ASIC_(REG)              ELB_ ##REG

void counterspd_show_global_counters(const int flags);
void counterspd_show_port_counters(const int port, const int flags);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEUTILPD_H__ */
