
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __DTLS_H__
#define __DTLS_H__

#define ACTIVITY_TIMEO_US   1000000     // After 1s of inactivity, sleep
#define INACTIVITY_SLEEP_US 20000       // Sleep for 20ms while inactive

/*
 * shared memory page
 */

#define MEMTUN_MAGIC        0xf3e42aad  // random
#define MEMTUN_VERSION      1
#define PKTS_PER_RING       64
#define MAX_PKTSIZE         1534        // 0x600 - 4 (for len)
#define INFO_MAXSIZE        1024

typedef struct {
    uint32_t len;
    uint8_t  data[MAX_PKTSIZE];         // data is word aligned
} pkt_t;

typedef struct {
    uint32_t    posted_index;
    uint32_t    fetch_index __attribute__((aligned(64)));
    pkt_t       ring[PKTS_PER_RING] __attribute__((aligned(64)));
} pkt_queue_t;

typedef struct {
    // written by capri, read by host
    uint32_t    magic;          // MEMTUN_MAGIC
    uint32_t    version;        // version
    uint32_t    memsize;        // sizeof (memtun_t)
    uint32_t    ring_size;      // PKTS_PER_RING
    uint32_t    txq_offs;       // offsetof txq
    uint32_t    rxq_offs;       // offsetof rxq
    uint32_t    info_offs;      // info table offset
    uint32_t    ackno;          // ack number

    // written by host, read by capri
    uint32_t    host_addr __attribute__((aligned(64))); // host IP address
    uint32_t    capri_addr;     // capri IP address
    uint32_t    seqno;          // seq number
} memhdr_t;

typedef struct {
    // header
    memhdr_t hdr;

    // rings
    pkt_queue_t txq __attribute__((aligned(64))); // host->capri
    pkt_queue_t rxq __attribute__((aligned(64))); // capri->host

    // info table
    char info[INFO_MAXSIZE] __attribute__((aligned(64)));
} memtun_t;

#define offsetof(t, f)          (intptr_t)&(((t *)0)->f)

typedef struct {
    volatile memhdr_t *hdr;
    volatile pkt_queue_t *txq;
    volatile pkt_queue_t *rxq;
} memtun_info_t;

/*
 * tun.c
 */
int tun_open(char **namp);
int tun_setaddrs(const char *ifname, uint32_t local, uint32_t remote);

/*
 * util.c
 */
void dump(int indent, const uint8_t *buf, uint32_t len);
int open_mem(const char *path, off_t offset, size_t size, void **memp);
void w_memcpy(void *dst, const void *src, size_t len);
void w_memzero(void *dst, size_t len);


/*
 * queue.c
 */
void txq_send(uint32_t ring_size, volatile pkt_queue_t *qp, const pkt_t *pkt);
int rxq_recv(uint32_t ring_size, volatile pkt_queue_t *qp, pkt_t *pkt);

/*
 * pktloop.c
 */
typedef int (*packet_pollcb_t)(int tunfd, memtun_info_t *mi,
        int *activity, void *arg);
int packet_loop(int tunfd, memtun_info_t *mi,
        packet_pollcb_t pollcb, void *arg);

#endif
