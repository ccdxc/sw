
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <system_error>
#include "dtls.h"
#include <signal.h>
#include <setjmp.h>
#include "dev.h"

static uint8_t tx;

static sigjmp_buf guard_jmpbuf;
static sighandler_t guard_osigbus;
static sighandler_t guard_osigsegv;

static void
sig_catcher(int sig)
{
    longjmp(guard_jmpbuf, 1);
}

static void
guardian_begin(void)
{
    guard_osigbus = signal(SIGBUS, sig_catcher);
    guard_osigsegv = signal(SIGSEGV, sig_catcher);

    if (sigsetjmp(guard_jmpbuf, 1)) {
        fflush(stdout);
        signal(SIGBUS, guard_osigbus);
        signal(SIGSEGV, guard_osigsegv);
        throw dev_io_exception();
    }
}

static void
guardian_end(void)
{
    signal(SIGBUS, guard_osigbus);
    signal(SIGSEGV, guard_osigsegv);
}

void
dev_read(uint32_t *wp, uint64_t addr, int count)
{
    uint64_t raddr;
    
    guardian_begin();
#ifdef __aarch64__
    raddr = addr;
    for (int i = 0; i < count; i++) {
        wp[i] = regio_readl(raddr);
        raddr += 4;
    }
#else
    for (int i = 0; i < count; i++) {
        wp[i] = 0xdeadbeef;
    }
#endif
    raddr = addr;
    for (int i = 0; i < count; i++) {
        logf("%02x RD: [%08lx] => %08x\n", tx, raddr, wp[i]);
        raddr += 4;
    }
    ++tx;
    guardian_end();
}

void
dev_write(const uint32_t *wp, uint64_t addr, int count)
{
    uint64_t waddr;

    guardian_begin();
#ifdef __aarch64__
    waddr = addr;
    for (int i = 0; i < count; i++) {
        regio_writel(waddr, wp[i]);
        waddr += 4;
    }
#endif
    waddr = addr;
    for (int i = 0; i < count; i++) {
        logf("%02x WR: [%08lx] <= %08x\n", tx, waddr, wp[i]);
        waddr += 4;
    }
    ++tx;
    guardian_end();
}
