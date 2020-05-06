/*
 * Copyright (c) 2018-2020, Pensando Systems Inc.
 */

#ifndef __PCIEPORT_H__
#define __PCIEPORT_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include "platform/pciemgr/include/pciemgr_params.h"
#include "pcieport_events.h"
#include "pcieport_stats.h"
#include "serdes.h"

/* XXX ELBA-TODO - some unit tests still use #ifdefs */
#ifdef ASIC_CAPRI
#include "capri/pcieportpd.h"
#endif
#ifdef ASIC_ELBA
#include "elba/pcieportpd.h"
#endif

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
        u_int8_t open;
        u_int16_t lanemask;
        u_int16_t subvendorid;
        u_int16_t subdeviceid;
        u_int32_t init:1;
        u_int32_t _spare:1;
        u_int32_t host:1;
        u_int32_t config:1;
        u_int32_t crs:1;
        u_int32_t compliance:1;
        u_int32_t aer_common:1;
        u_int32_t sris:1;
        u_int32_t vga_support:1;
        u_int32_t reduce_rx_cred:1;
        u_int32_t cur_reversed:1;
        pcieportst_t state;
        pcieportev_t event;
        char fault_reason[80];
        char last_fault_reason[80];
        pcieport_stats_t stats;
        u_int32_t macup;
        int txfc_phdr;                  /* tx fc credits    posted hdr */
        int txfc_pdata;                 /* tx fc credits    posted data */
        int txfc_nphdr;                 /* tx fc credits nonposted hdr */
        int txfc_npdata;                /* tx fc credits nonposted data */
        u_int32_t clock_freq;           /* core clock frequency */
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
    extern pcieport_info_t *
    pcieport_info_get_or_map(pciemgr_initmode_t initmode);

    return pcieport_info_get_or_map(INHERIT_ONLY);
}

static inline pcieport_t *
pcieport_get(const int port)
{
    pcieport_info_t *pi = pcieport_info_get();
    pcieport_t *p = NULL;

    if (pi != NULL && port >= 0 && port < PCIEPORT_NPORTS) {
        p = &pi->pcieport[port];
    }
    return p;
}

int pcieport_open(const int port, pciemgr_initmode_t initmode);
void pcieport_close(const int port);

int pcieport_hostconfig(const int port, const pciemgr_params_t *params);
int pcieport_crs_off(const int port);
int pcieport_is_accessible(const int port);
int pcieport_powerdown(const int port);
void pcieport_get_tx_fc_credits(const int port,
                                int *posted_data, int *posted_hdr,
                                int *nonposted_data, int *nonposted_hdr);

int pcieport_intr_init(const int port);
int pcieport_intr(const int port);
int pcieport_poll_init(const int port);
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
int pcieport_get_recovery(const int port);

const char *pcieport_stname(const pcieportst_t st);
const char *pcieport_evname(const pcieportev_t ev);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEPORT_H__ */
