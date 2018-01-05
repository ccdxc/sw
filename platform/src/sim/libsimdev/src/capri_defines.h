#ifndef _CAPRI_DEFINES_H_
#define _CAPRI_DEFINES_H_

#define PACKED __attribute__((packed))

#define CAPRI_DB_ADDR_SCHED_UPD_NO_CHG          0x0
#define CAPRI_DB_ADDR_SCHED_UPD_EVAL            0x1
#define CAPRI_DB_ADDR_SCHED_UPD_CLEAR           0x2
#define CAPRI_DB_ADDR_SCHED_UPD_SET             0x3

#define CAPRI_DB_ADDR_IDX_NO_UPD                0x0
#define CAPRI_DB_ADDR_IDX_CIDX_SET              0x1
#define CAPRI_DB_ADDR_IDX_PIDX_SET              0x2
#define CAPRI_DB_ADDR_IDX_PIDX_INCR             0x3

#define CAPRI_DB_ADDR_PID_CHECK                 0x1
#define CAPRI_DB_ADDR_PID_CHECK_NONE            0x0

typedef struct capri_db_addr_s {
    uint64_t        rsvd1:3;    /* DWORD Alignment */
    uint64_t        type:3;
    uint64_t        lif:11;
    uint64_t        upd_sched_upd:2;
    uint64_t        upd_idx_update:2;
    uint64_t        pid_check:1;
} PACKED capri_db_addr_t;

typedef struct capri_db_data_s {
    uint64_t        index:16;
    uint64_t        ring:3;
    uint64_t        rsvd:5;
    uint64_t        qid:24;
    uint64_t        pid:16;
} PACKED capri_db_data_t;

static inline uint64_t capri_doorbell_address(uint8_t pid_check,
        uint8_t idx_update, uint8_t sched_update, uint16_t lif,
        uint8_t type)
{
    union {
        capri_db_addr_t     db_addr;
        uint64_t            u64;
    } dba;

    dba.u64 = 0;

    dba.db_addr.pid_check = pid_check;
    dba.db_addr.upd_idx_update = idx_update;
    dba.db_addr.upd_sched_upd = sched_update;
    //dba.db_addr.lif = lif;
    dba.db_addr.type = type;
    simdev_log("%s: Posting dbell: LIF:%d, Type:%d, Addr: 0x%lx\n", 
            __FUNCTION__, lif, type, dba.u64);

    return dba.u64;
}


static inline uint64_t capri_doorbell_data(uint16_t pid, uint32_t qid,
        uint8_t ring, uint16_t index)
{
    union {
        capri_db_data_t     db_data;
        uint64_t            u64;
    } dbd;

    dbd.u64 = 0;

    dbd.db_data.pid = pid;
    dbd.db_data.qid = qid;
    dbd.db_data.ring = ring;
    dbd.db_data.index = index;

    return dbd.u64;
}

#endif /* _CAPRI_DEFINES_H_ */
