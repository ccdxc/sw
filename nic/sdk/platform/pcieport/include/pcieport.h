/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PCIEPORT_H__
#define __PCIEPORT_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include "pcieport_events.h"
#include "pcieport_stats.h"

#define PCIEPORT_VERSION        1
#define PCIEPORT_NPORTS         8

typedef enum pcieportst_e {
    PCIEPORTST_OFF,
    PCIEPORTST_DOWN,
    PCIEPORTST_MACUP,
    PCIEPORTST_LINKUP,
    PCIEPORTST_UP,
    PCIEPORTST_FAULT,

    PCIEPORTST_MAX
} pcieportst_t;

typedef enum pcieportev_e {
    PCIEPORTEV_MACDN,
    PCIEPORTEV_MACUP,
    PCIEPORTEV_LINKDN,
    PCIEPORTEV_LINKUP,
    PCIEPORTEV_BUSCHG,
    PCIEPORTEV_POWERDN,

    PCIEPORTEV_MAX
} pcieportev_t;

union pcieport_u {
    struct {
        int port;
        int cap_gen;
        int cap_width;
        int cur_gen;
        int cur_width;
        int req_gen;
        int req_width;
        u_int8_t pribus;
        u_int8_t secbus;
        u_int8_t subbus;
        u_int16_t lanemask;
        u_int16_t subvendorid;
        u_int16_t subdeviceid;
        u_int32_t init:1;
        u_int32_t open:1;
        u_int32_t host:1;
        u_int32_t config:1;
        u_int32_t crs:1;
        u_int32_t compliance:1;
        u_int32_t aer_common:1;
        u_int32_t sris:1;
        u_int32_t vga_support:1;
        u_int32_t reduce_rx_cred:1;
        pcieportst_t state;
        pcieportev_t event;
        char fault_reason[80];
        char last_fault_reason[80];
        pcieport_stats_t stats;
    };
    u_int8_t _pad[2048];
};
typedef union pcieport_u pcieport_t;

struct pcieport_info_s {
    u_int32_t version;
    u_int32_t init:1;
    u_int32_t serdes_init:1;
    u_int32_t serdes_init_always:1;
    u_int32_t inherited_init:1;
    pcieport_t pcieport[PCIEPORT_NPORTS];
};
typedef struct pcieport_info_s pcieport_info_t;

static inline pcieport_info_t *
pcieport_info_get(void)
{
    extern pcieport_info_t *pcieport_info;
    return pcieport_info;
}

int pcieport_open(const int port, pciemgr_initmode_t initmode);
void pcieport_close(const int port);

struct pciemgr_params_s;
typedef struct pciemgr_params_s pciemgr_params_t;

int pcieport_hostconfig(const int port, const pciemgr_params_t *params);
int pcieport_crs_off(const int port);
int pcieport_is_accessible(const int port);
int pcieport_powerdown(const int port);

int pcieport_poll(const int port);
void pcieport_dbg(int argc, char *argv[]);

void pcieport_showport(const int port);
void pcieport_showports(void);

/* flags for showportstats() */
#define PSF_NONE 0x0
#define PSF_ALL  0x1
void pcieport_showportstats(const int port, const unsigned int flags);
void pcieport_clearportstats(const int port, const unsigned int flags);
pcieport_stats_t *pcieport_stats_get(const int port);

const char *pcieport_stname(const pcieportst_t st);
const char *pcieport_evname(const pcieportev_t ev);

/*
 * Register convenience macros.
 */
#define PP_(REG) \
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

/* sta_mac flags */
#define STA_MACF_(REG) \
    (CAP_PXC_CSR_STA_C_PORT_MAC_ ##REG## _FIELD_MASK)

/* cfg_mac flags */
#define CFG_MACF_(REG) \
    (CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_ ##REG## _FIELD_MASK)

/* mac_intreg flags */
#define MAC_INTREGF_(REG) \
    (CAP_PXC_CSR_INT_C_MAC_INTREG_ ##REG## _INTERRUPT_FIELD_MASK)

#define INTREG_PERST0N \
    CAP_PP_CSR_INT_PP_INTREG_PERST0N_DN2UP_INTERRUPT_FIELD_MASK
#define INTREG_PERSTN(port) \
    (INTREG_PERST0N >> (port))

#ifdef __cplusplus
}
#endif

#endif /* __PCIEPORT_H__ */
