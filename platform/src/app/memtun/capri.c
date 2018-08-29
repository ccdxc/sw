
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include "dtls.h"

static char *progname;

typedef struct {
    int last_seqno;
    char *ifname;
} pollstate_t;

static int
cmd_poll(int tunfd, memtun_info_t *mi, int *activity, void *arg)
{
    pollstate_t *ps = (pollstate_t *)arg;

    // Check for a reset
    if (mi->hdr->seqno != ps->last_seqno) {
        if (tun_setaddrs(ps->ifname, mi->hdr->capri_addr,
                mi->hdr->host_addr) < 0) {
            return -1;
        }
        mi->txq->posted_index = mi->txq->fetch_index = 0;
        mi->rxq->posted_index = mi->rxq->fetch_index = 0;
        mi->hdr->ackno = mi->hdr->seqno;
        ps->last_seqno = mi->hdr->seqno;
        *activity = 1;
    }
    return 0;
}

static int
mem_init(memtun_info_t *mi, off_t phys, char *info, uint32_t infosz)
{
    static const char path[] = "/dev/mem";
    memtun_t *mp;
    void *mem;

    if (open_mem(path, phys, sizeof (memtun_t), &mem) < 0) {
        return -1;
    }
    mp = (memtun_t *)mem;

    w_memzero(&mp->hdr, sizeof (mp->hdr));
    mp->hdr.version = MEMTUN_VERSION;
    mp->hdr.memsize = sizeof (memtun_t);
    mp->hdr.ring_size = PKTS_PER_RING;
    mp->hdr.txq_offs = offsetof(memtun_t, txq);
    mp->hdr.rxq_offs = offsetof(memtun_t, rxq);
    mp->hdr.info_offs = offsetof(memtun_t, info);

    w_memcpy(mp->info, info, infosz);

    mp->txq.posted_index = 0;
    mp->txq.fetch_index = 0;
    mp->rxq.posted_index = 0;
    mp->rxq.fetch_index = 0;

    mp->hdr.magic = MEMTUN_MAGIC;

    mi->hdr = &mp->hdr;
    mi->txq = &mp->rxq;
    mi->rxq = &mp->txq;
    return 0;
}

static int
memtun(off_t phys, char *info, uint32_t infosz)
{
    pollstate_t st;
    memtun_info_t mi;
    int tunfd;

    tunfd = tun_open(&st.ifname);
    if (tunfd < 0) {
        return -1;
    }

    if (mem_init(&mi, phys, info, infosz) < 0) {
        return -1;
    }

    // Packet loop
    st.last_seqno = mi.hdr->seqno;
    return packet_loop(tunfd, &mi, cmd_poll, &st);
}

static int
usage(void)
{
    fprintf(stderr, "usage: %s physaddr [name=value...]\n", progname);
    return 1;
}

int
main(int argc, char *argv[])
{
    uint32_t infosz;
    char *info, *wp;
    int i, pass;
    off_t phys;
    char *p;

    progname = argv[0];

    if (argc < 2) {
        return usage();
    }
    phys = strtoull(argv[1], &p, 0);
    if (*p != '\0') {
        return usage();
    }
    infosz = 0;
    for (pass = 1; pass <= 2; pass++) {
        for (i = 2; i < argc; i++) {
            if (pass == 1) {
                p = strchr(argv[i], '=');
                if (p == NULL) {
                    return usage();
                }
                infosz += strlen(argv[i]) + 1;
            } else {
                memcpy(wp, argv[i], strlen(argv[i]) + 1);
                wp += strlen(argv[i]) + 1;
            }
        }
        if (pass == 1) {
            ++infosz;
            if (infosz > INFO_MAXSIZE) {
                fprintf(stderr, "info too long (%u/%u bytes)\n",
                        infosz, INFO_MAXSIZE);
                return 1;
            }
            wp = info = malloc(infosz);
        } else {
            *wp++ = '\0';
        }
    }

    return memtun(phys, info, infosz);
}
