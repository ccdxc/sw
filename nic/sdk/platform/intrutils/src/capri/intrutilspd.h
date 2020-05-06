/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#ifndef __INTRUTILSPD_H__
#define __INTRUTILSPD_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include "cap_top_csr_defines.h"
#include "cap_intr_c_hdr.h"

#define ASIC_(REG)              CAP_ ##REG
#define INTR_BASE               ASIC_(ADDR_BASE_INTR_INTR_OFFSET)

void intrpd_hwinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __INTRUTILSPD_H__ */
