
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <sys/mman.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "dtls.h"

static char *progname;

static int
mem_init(memtun_info_t *mi, off_t phys)
{
    static const char path[] = "/dev/mem";
    memtun_t *mp;
    void *mem;

    if (open_mem(path, phys, sizeof (memtun_t), &mem) < 0) {
        return -1;
    }
    mp = (memtun_t *)mem;
    if (mp->hdr.magic != MEMTUN_MAGIC) {
        fprintf(stderr, "invalid magic\n");
        return -1;
    }
    if (mp->hdr.memsize > sizeof (memtun_t)) {
        munmap(mem, sizeof (memtun_t));
        if (open_mem(path, phys, mp->hdr.memsize, &mem) < 0) {
            return -1;
        }
        mp = (memtun_t *)mem;
    }
    mi->hdr = &mp->hdr;
    mi->txq = (pkt_queue_t *)((char *)mp + mp->hdr.txq_offs);
    mi->rxq = (pkt_queue_t *)((char *)mp + mp->hdr.rxq_offs);
    return 0;
}

static int
memtun(off_t phys, uint32_t host_addr)
{
    uint32_t capri_addr = host_addr + 1;
    memtun_info_t mi;
    char *ifname;
    int tunfd;

    tunfd = tun_open(&ifname);
    if (tunfd < 0) {
        return -1;
    }
    if (tun_setaddrs(ifname, host_addr, capri_addr) < 0) {
        return -1;
    }

    if (mem_init(&mi, phys) < 0) {
        return -1;
    }

    // Send the tunnel endpoint addresses to capri and wait for the ack.
    mi.hdr->host_addr = host_addr;
    mi.hdr->capri_addr = capri_addr;
    asm volatile("" ::: "memory");
    mi.hdr->seqno++;
    while (mi.hdr->ackno != mi.hdr->seqno) {
        ; /* wait for ack */
    }

    // Packet loop
    return packet_loop(tunfd, &mi, NULL, NULL);
}

static int
usage(void)
{
    fprintf(stderr, "usage: %s physaddr local_ip\n", progname);
    return 1;
}

int
main(int argc, char *argv[])
{
    struct in_addr ipaddr;
    off_t phys;
    char *p;

    progname = argv[0];

    if (argc != 3) {
        return usage();
    }
    phys = strtoull(argv[1], &p, 0);
    if (*p != '\0') {
        return usage();
    }
    if (inet_aton(argv[2], &ipaddr) < 0) {
        return usage();
    }

    return memtun(phys, ntohl(ipaddr.s_addr));
}
