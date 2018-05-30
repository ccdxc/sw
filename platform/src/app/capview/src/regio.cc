
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <system_error>
#include "dtls.h"

static int regio_fd;

#define REGIO_LOG2_PGSZ     24
#define REGIO_PGSZ          (1LL << REGIO_LOG2_PGSZ)

static void
regio_init(void)
{
    static const char path[] = "/dev/mem";
    static bool init_done;

    if (!init_done) {
        regio_fd = open(path, O_RDWR, 0);
        if (regio_fd < 0) {
            throw std::system_error(errno, std::system_category(), path);
        }
        if (sysconf(_SC_PAGESIZE) > REGIO_PGSZ) {
            throw std::runtime_error("REGIO_PGSZ too small");
        }
        init_done = true;
    }
}

static void *
regio_p2v(uint64_t addr)
{
    static std::map<uint64_t, void *> pagemap;
    static uint64_t last_base = 1;
    static void *last_pg;
    uint64_t base = addr & -REGIO_PGSZ;
    void *pg;

    if (base == last_base) {
        pg = last_pg;
    } else {
        regio_init();
        auto it = pagemap.find(base);
        if (it == pagemap.end()) {
            pg = mmap(NULL, REGIO_PGSZ, PROT_READ | PROT_WRITE,
                    MAP_SHARED, regio_fd, base);
            if (pg == (void *)-1) {
                throw std::system_error();
            }
            pagemap[base] = pg;
        } else {
            pg = it->second;
        }
    }
    logf("PA(0x%lx) => VA(%p)\n", base, pg);
    last_base = base;
    last_pg = pg;
    uint64_t offs = addr & (REGIO_PGSZ - 1);
    return (void *)((char *)pg + offs);
}

uint32_t
regio_readl(uint64_t addr)
{
    if (addr & 0x3) {
        throw std::runtime_error("non-word-aligned address");
    }
    return *(uint32_t *)regio_p2v(addr);
}

void
regio_writel(uint64_t addr, uint32_t val)
{
    if (addr & 0x3) {
        throw std::runtime_error("non-word-aligned address");
    }
    *(uint32_t *)regio_p2v(addr) = val;
}
