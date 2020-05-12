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

#include "elb_top_csr_defines.h"
#include "elb_intr_c_hdr.h"

#define ASIC_(REG)              ELB_ ##REG
#define INTR_BASE               ASIC_(ADDR_BASE_INTR_INTR_OFFSET)

#define INTR_PBA_OFFSET         ASIC_(INTR_CSR_DHS_INTR_PBA_ARRAY_BYTE_OFFSET)
#define INTR_PBA_BASE           (INTR_BASE + INTR_PBA_OFFSET)
#define INTR_PBA_STRIDE         0x10

void intrpd_hwinit(const u_int32_t clock_freq);
void intrpd_coal_init(const u_int32_t clock_freq);

#ifdef __cplusplus
}
#endif

#endif /* __INTRUTILSPD_H__ */
