/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#ifndef __CAPRI_PCIEPORTPD_H__
#define __CAPRI_PCIEPORTPD_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"

/*
 * Register convenience macros.
 */
#define PP_(REG, pn) \
    (CAP_ADDR_BASE_PP_PP_OFFSET + CAP_PP_CSR_ ##REG## _BYTE_ADDRESS)

#define PXB_(REG) \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_ ##REG## _BYTE_ADDRESS)

#define PXC_(REG, pn) \
    (CAP_ADDR_BASE_PP_PP_OFFSET + \
     ((pn) * CAP_PXC_CSR_BYTE_SIZE) + \
     CAP_PP_CSR_PORT_C_ ##REG## _BYTE_ADDRESS)

#define PXP_(REG, pn) \
    (CAP_ADDR_BASE_PP_PP_OFFSET + \
     ((pn) * CAP_PXP_CSR_BYTE_SIZE) + \
     CAP_PP_CSR_PORT_P_ ##REG## _BYTE_ADDRESS)

/* sta_rst flags */
#define STA_RSTF_(REG) \
    (CAP_PXC_CSR_STA_C_PORT_RST_ ##REG## _FIELD_MASK)

/* sta_c_port_mac flags */
#define STA_C_PORT_MACF_(REG) \
    (CAP_PXC_CSR_STA_C_PORT_MAC_ ##REG## _FIELD_MASK)

/* sta_p_port_mac flags */
#define STA_P_PORT_MACF_(REG) \
    (CAP_PXP_CSR_STA_P_PORT_MAC_ ##REG## _FIELD_MASK)

/* cfg_mac flags */
#define CFG_MACF_(REG) \
    (CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_ ##REG## _FIELD_MASK)

/* mac_intreg flags */
#define MAC_INTREGF_(REG) \
    (CAP_PXC_CSR_INT_C_MAC_INTREG_ ##REG## _INTERRUPT_FIELD_MASK)

#define PP_INTREG_PERST0N \
    CAP_PP_CSR_INT_PP_INTREG_PERST0N_DN2UP_INTERRUPT_FIELD_MASK
#define PP_INTREG_PERSTN(port) \
    (PP_INTREG_PERST0N >> (port))

#define PP_INTREG_PORT0_C_INT_INTERRUPT \
    CAP_PP_CSR_INT_PP_INTREG_PORT0_C_INT_INTERRUPT_FIELD_MASK
#define PP_INTREG_PORT_C_INT_INTERRUPT(port) \
    (PP_INTREG_PORT0_C_INT_INTERRUPT >> ((port) * 2))

#define CFG_C_PORT_MAC_F_MAC_RESET \
    CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_RESET_FIELD_MASK

#define CFG_C_PORT_MAC_F_LTSSM_EN \
    CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_LTSSM_EN_FIELD_MASK

#define CFG_C_PORT_MAC_F_AER_COMMON_EN \
    CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_1_2_AER_COMMON_EN_FIELD_MASK

#define CFG_C_PORT_MAC_F_CLOCK_FREQ_MASK \
    CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_TL_CLOCK_FREQ_FIELD_MASK
#define CFG_C_PORT_MAC_F_CLOCK_FREQ_SHIFT \
    CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_TL_CLOCK_FREQ_LSB

#ifdef __cplusplus
}
#endif

#endif /* __CAPRI_PCIEPORTPD_H__ */
