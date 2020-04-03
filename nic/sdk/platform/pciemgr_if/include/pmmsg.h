/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PMMSG_H__
#define __PMMSG_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

typedef enum pmmsg_type_e {
    PMMSG_OPEN,
    PMMSG_INITIALIZE,
    PMMSG_DEVRES_ADD,
    PMMSG_FINALIZE,
    PMMSG_EVENT,
    PMMSG_POWERMODE,
} pmmsgtype_t;

typedef struct pmmsg_hdr_s {
    u_int16_t msgtype;
    u_int16_t msglen;
    struct pmmsg_s *link;
    void *tag;
} pmmsg_hdr_t;

typedef struct pmmsg_open_s {
    char name[32];
    int receiver;
} pmmsg_open_t;

typedef struct pmmsg_initialize_s {
    int port;
} pmmsg_initialize_t;

typedef struct pmmsg_finalize_s {
    int port;
} pmmsg_finalize_t;

typedef struct pmmsg_dev_add_s {
    int unused;
    /*
     * pciehdev_t
     * pciehcfg_t
     * pciehbar[0]
     *     pciehreg[0]...[n]
     * ...
     * pciehbar[n]
     *     pciehreg[0]...[n]
     */
} pmmsg_dev_add_t;

typedef struct pmmsg_devres_add_s {
    int unused;
    /*
     * pciehdevice_resources_t
     */
} pmmsg_devres_add_t;

typedef struct pmmsg_event_s {
    int unused;
    /*
     * pciehdev_eventdata_t
     */
} pmmsg_event_t;

typedef struct pmmsg_powermode_s {
    int mode;
} pmmsg_powermode_t;

typedef struct pmmsg_s {
    pmmsg_hdr_t hdr;
    union {
        char msgdata;
        pmmsg_open_t open;
        pmmsg_finalize_t finalize;
        pmmsg_initialize_t initialize;
        pmmsg_dev_add_t dev_add;
        pmmsg_devres_add_t devres_add;
        pmmsg_event_t event;
        pmmsg_powermode_t powermode;
    };
} pmmsg_t;

int pciemgr_msgalloc(pmmsg_t **m, size_t len);
void pciemgr_msgfree(pmmsg_t *m);

#ifdef __cplusplus
}
#endif

#endif /* __PMMSG_H__ */
