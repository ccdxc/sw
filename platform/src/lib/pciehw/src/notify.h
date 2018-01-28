/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __NOTIFY_H__
#define __NOTIFY_H__

typedef struct notify_info_s {
    /* u_int64_t [0] */
    u_int64_t direct_endaddr    :6;
    u_int64_t direct_blen       :4;
    u_int64_t is_indirect       :1;
    u_int64_t is_direct         :1;
    u_int64_t is_UR             :1;
    u_int64_t is_CA             :1;
    u_int64_t romsksel          :7;
    u_int64_t context_id        :7;
    u_int64_t vfid              :11;
    u_int64_t is_notify         :1;
    u_int64_t direct_size       :9;
    /* u_int64_t [1] */
    u_int64_t direct_addr       :52;
    u_int64_t aspace            :1;
    u_int64_t pmti              :10;
    u_int64_t pmt_hit           :1;
    u_int64_t indirect_reason   :5;
    u_int64_t is_host           :1;
    u_int64_t ax_len            :4;
    u_int64_t rsrv              :2;
    u_int64_t eop               :1;
    u_int64_t sop               :1;
} __attribute__((packed)) notify_info_t;

#define NOTIFY_TLPSZ            48
#define NOTIFY_INFOSZ           sizeof(notify_info_t)

typedef struct notify_entry_s {
    u_int8_t rtlp[NOTIFY_TLPSZ];
    notify_info_t info;
} notify_entry_t;

#define NOTIFY_ENTRYSZ          sizeof(notify_entry_t)
#define NOTIFY_NENTRIES         0x4000  /* power of 2 */
#define NOTIFYSZ                (NOTIFY_NENTRIES * NOTIFY_ENTRYSZ)

struct pciehw_s;
typedef struct pciehw_s pciehw_t;

int pciehw_notify_init(pciehw_t *phw);
int pciehw_notify_poll(pciehw_t *phw);
void pciehw_notify_dbg(int argc, char *argv[]);

#endif /* __NOTIFY_H__ */
