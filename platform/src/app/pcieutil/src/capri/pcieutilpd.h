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

#include "cap_wa_c_hdr.h"

#define ASIC_CAPRI
#include "platform/pciemgr/include/pciemgr.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "platform/pcieport/include/pcieport.h"
#include "platform/pcieport/include/capri/pcieportpd.h"
#include "platform/pciemgr/include/pciehw.h"
#include "platform/pciemgr/include/pciehw_dev.h"

#define ASIC_(REG)              CAP_ ##REG

void counterspd_show_global_counters(const int flags);
void counterspd_show_port_counters(const int port, const int flags);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEUTILPD_H__ */
