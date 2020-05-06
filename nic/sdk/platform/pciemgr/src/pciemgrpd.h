/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#ifndef __PCIEMGRPD_H__
#define __PCIEMGRPD_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#ifdef ASIC_CAPRI
/******************************************************************/
#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"

#define ASIC_(REG)      CAP_ ##REG
#define PXB_(REG) \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_ ##REG## _BYTE_ADDRESS)

#endif
#ifdef ASIC_ELBA
/******************************************************************/
#include "elb_top_csr_defines.h"
#include "elb_pxb_c_hdr.h"

#define ASIC_(REG)      ELB_ ##REG
#define PXB_(REG) \
    (ELB_ADDR_BASE_PXB_PXB_OFFSET + ELB_PXB_CSR_ ##REG## _BYTE_ADDRESS)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PCIEMGRPD_H__ */
