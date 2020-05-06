/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#ifndef __ELBA_PCIEPORTPD_H__
#define __ELBA_PCIEPORTPD_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include "elb_top_csr_defines.h"
#include "elb_pp_c_hdr.h"
#include "elb_pxb_c_hdr.h"
#include "elb_pxc_c_hdr.h"

/*
 * Register convenience macros.
 */
#define _PP_BASE(pn) \
    (ELB_ADDR_BASE_PP_PP_0_OFFSET + \
     (((pn) >> 2) * ELB_ADDR_BASE_PP_PP_0_SIZE))

#define PP_(REG, pn) \
    (_PP_BASE(pn) + ELB_PP_CSR_ ##REG## _BYTE_ADDRESS)

#define PXB_(REG) \
    (ELB_ADDR_BASE_PXB_PXB_OFFSET + ELB_PXB_CSR_ ##REG## _BYTE_ADDRESS)

#define _PXC_BASE(pn) \
    (ELB_ADDR_BASE_PP_PXC_0_OFFSET + \
     ((pn) * ELB_ADDR_BASE_PP_PXC_0_SIZE))

#define PXC_(REG, pn) \
    (_PXC_BASE(pn) + ELB_PXC_CSR_ ##REG## _BYTE_ADDRESS)

#define PXP_(REG, pn) \
    (_PXC_BASE(pn) + ELB_PXC_CSR_PORT_P_ ##REG## _BYTE_ADDRESS)

/* sta_rst flags */
#define STA_RSTF_(REG) \
    (ELB_PXC_CSR_STA_C_PORT_RST_ ##REG## _FIELD_MASK)

/* sta_c_port_mac flags */
#define STA_C_PORT_MACF_(REG) \
    (ELB_PXC_CSR_STA_C_PORT_MAC_ ##REG## _FIELD_MASK)

/* sta_p_port_mac flags */
#define STA_P_PORT_MACF_(REG) \
    (ELB_PXP_CSR_STA_P_PORT_MAC_ ##REG## _FIELD_MASK)

/* cfg_mac flags */
#define CFG_MACF_(REG) \
    (ELB_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_ ##REG## _FIELD_MASK)

/* mac_intreg flags */
#define MAC_INTREGF_(REG) \
    (ELB_PXC_CSR_INT_C_MAC_INTREG_ ##REG## _INTERRUPT_FIELD_MASK)

#define PP_INTREG_PERST0N \
    ELB_PP_CSR_INT_PP_INTREG_PERST0N_DN2UP_INTERRUPT_FIELD_MASK
#define PP_INTREG_PERSTN(port) \
    (PP_INTREG_PERST0N >> (port))

#define PP_INTREG_PORT0_C_INT_INTERRUPT \
    ELB_PP_CSR_INT_PP_INTREG_PORT0_C_INT_INTERRUPT_FIELD_MASK
#define PP_INTREG_PORT_C_INT_INTERRUPT(port) \
    (PP_INTREG_PORT0_C_INT_INTERRUPT >> ((port) * 2))

#define CFG_C_PORT_MAC_F_MAC_RESET \
    ELB_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_RESET_FIELD_MASK

#define CFG_C_PORT_MAC_F_LTSSM_EN \
    ELB_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_LTSSM_EN_FIELD_MASK

#define CFG_C_PORT_MAC_F_AER_COMMON_EN \
    ELB_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_1_2_AER_COMMON_EN_FIELD_MASK

#define CFG_C_PORT_MAC_F_CLOCK_FREQ_MASK \
    ELB_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_TL_CLOCK_FREQ_FIELD_MASK
#define CFG_C_PORT_MAC_F_CLOCK_FREQ_SHIFT \
    ELB_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_TL_CLOCK_FREQ_LSB

#ifdef __cplusplus
}
#endif

#endif /* __ELBA_PCIEPORTPD_H__ */
