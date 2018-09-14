
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
#include <getopt.h>
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
    fprintf(stderr,
            "usage: %s [-d vvvv:dddd | -s bb:dd.f | -p physaddr] local_ip\n",
            progname);
    return 1;
}

static u_int32_t
setpci_cmd(const char *cmd)
{
    char line[40];
    FILE *fp;
    u_int32_t phys = 0;

    fp = popen(cmd, "r");
    if (fp != NULL) {
        line[0] = '0';
        line[1] = 'x';
        while (fgets(line + 2, sizeof(line) - 2, fp) != NULL) {
            phys = strtoul(line, NULL, 0);
            if (phys) break;
        }
        pclose(fp);
    }
    return phys;
}

static u_int32_t
setpci_str(const char *flag, const char *str, const char *reg)
{
    char cmd[80];
    snprintf(cmd, sizeof(cmd), "setpci %s %s %s", flag, str, reg);
    return setpci_cmd(cmd);
}

static off_t
get_phys_from_sstr(const char *sstr)
{
    const u_int64_t bar0 = setpci_str("-s", sstr, "base_address_0");
    const u_int64_t bar1 = setpci_str("-s", sstr, "base_address_1");
    return (bar1 << 32) | (bar0 & ~0xfULL);
}

static off_t
get_phys_from_dstr(const char *dstr)
{
    const u_int64_t bar0 = setpci_str("-d", dstr, "base_address_0");
    const u_int64_t bar1 = setpci_str("-d", dstr, "base_address_1");
    return (bar1 << 32) | (bar0 & ~0xfULL);
}

int
main(int argc, char *argv[])
{
    struct in_addr ipaddr;
    off_t phys;
    char *p, *dstr, *sstr;
    int opt;

    progname = argv[0];
    dstr = "1dd8:1001";
    sstr = NULL;
    phys = 0;
    while ((opt = getopt(argc, argv, "d:s:p:")) != -1) {
        switch (opt) {
        case 'd':
            dstr = optarg;
            break;
        case 's':
            sstr = optarg;
            break;
        case 'p':
            dstr = sstr = NULL;
            phys = strtoull(optarg, &p, 0);
            if (*p != '\0') {
                return usage();
            }
            break;
        default:
            return usage();
        }
    }

    if (sstr) {
        phys = get_phys_from_sstr(sstr);
    } else if (dstr) {
        phys = get_phys_from_dstr(dstr);
    }
    if (phys == 0) {
        return usage();
    }
    if (argc - optind != 1) {
        return usage();
    }
    if (inet_aton(argv[optind], &ipaddr) < 0) {
        return usage();
    }

    return memtun(phys, ntohl(ipaddr.s_addr));
}
