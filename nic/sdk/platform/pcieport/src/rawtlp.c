/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <endian.h>
#include <fcntl.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/param.h>

#include "cap_top_csr_defines.h"
#include "cap_pp_c_hdr.h"

#include "platform/pal/include/pal.h"
#include "platform/pcietlp/include/pcietlp.h"
#include "platform/pciemgr/include/pciemgr.h"
#include "pcieport.h"
#include "rawtlp.h"

static int rawtlp_lockfd = -1;

static int
rawtlp_lock(void)
{
    struct flock lockop = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
    };

    if (rawtlp_lockfd == -1) {
        rawtlp_lockfd = open("/var/lock/rawtlp.lck", O_CREAT | O_RDWR, 0666);
    }
    return fcntl(rawtlp_lockfd, F_SETLKW, &lockop);
}

static int
rawtlp_unlock(void)
{
    struct flock unlockop = {
        .l_type = F_UNLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
    };

    return fcntl(rawtlp_lockfd, F_SETLKW, &unlockop);
}

static int
rawtlp_req(const int port, const uint32_t *reqtlp, const size_t reqtlpsz)
{
    int i;

    const int ndw = roundup(reqtlpsz, 4) >> 2;
    if (ndw > 12) return -E2BIG;

    uint32_t reqw[12];
    for (i = 0; i < ndw; i++) {
        reqw[i] = htobe32(reqtlp[i]); // be words for hw
    }
    for ( ; i < 12; i++) {
        reqw[i] = htobe32(0); // zero unused words
    }

    pal_reg_wr32w(PXB_(CFG_ITR_RAW_TLP), reqw, 12);

    cfg_itr_raw_tlp_cmd_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.port_id = port;
    cmd.dw_cnt = ndw;
    cmd.cmd_go = 1;
    pal_reg_wr32(PXB_(CFG_ITR_RAW_TLP_CMD), cmd.w);
    return 0;
}

static void
rawtlp_sta_itr_raw_tlp(sta_itr_raw_tlp_t *sta)
{
    sta->w = pal_reg_rd32(PXB_(STA_ITR_RAW_TLP));
}

static int
rawtlp_resp_rdy(sta_itr_raw_tlp_t *sta)
{
    rawtlp_sta_itr_raw_tlp(sta);
#ifdef __aarch64__
    return sta->resp_rdy;
#else
    return 1;
#endif
}

static int
rawtlp_wait_rdy(sta_itr_raw_tlp_t *sta, const int timeout_us)
{
    int us = 0;

    do {
        if (rawtlp_resp_rdy(sta)) return us;
        usleep(1);
    } while (++us < timeout_us);
    return -ETIMEDOUT;
}

static int
rawtlp_rsp_data(uint32_t *rsptlp, const size_t rsptlpsz)
{
    const int ndw = rsptlpsz >> 2;
    if (ndw > 8) return -E2BIG;

    uint32_t rspw[8];
    pal_reg_rd32w(PXB_(STA_ITR_RAW_TLP_DATA), rspw, ndw);
    for (int i = 0; i < ndw; i++) {
        rsptlp[i] = be32toh(rspw[i]);
    }
    return 0;
}

int
rawtlp_send(const int port,
            const uint32_t *reqtlp, const size_t reqtlpsz,
            uint32_t *rspdata, const size_t rspdatasz,
            sta_itr_raw_tlp_t *sta)
{
    int r = 0;

    r = rawtlp_lock();
    if (r < 0) {
        pciesys_logerror("rawtlp%d lock failed: %s\n", port, strerror(errno));
        goto out;
    }

    r = rawtlp_wait_rdy(sta, 1000);
    if (r < 0) {
        pciesys_logerror("rawtlp%d not ready to start: %d\n", port, r);
        r = -EBUSY;
        goto out_unlock;
    }

    r = rawtlp_req(port, reqtlp, reqtlpsz);
    if (r < 0) {
        pciesys_logerror("rawtlp%d request failed: %d\n", port, r);
        goto out_unlock;
    }

    r = rawtlp_wait_rdy(sta, 5000000);
    if (r < 0) {
        pciesys_logerror("rawtlp%d failed waiting for completion: %d\n",
                         port, r);
        goto out_unlock;
    }

    if (rspdata) {
        r = rawtlp_rsp_data(rspdata, rspdatasz);
        if (r < 0) {
            pciesys_logerror("rawtlp%d failed reading response data: %d\n",
                             port, r);
            goto out_unlock;
        }
    }

 out_unlock:
    rawtlp_unlock();
 out:
    return r;

}

int
rawtlp_sta_errs(const int port, sta_itr_raw_tlp_t *sta)
{
    if (sta->cpl_data_err ||
        sta->cpl_timeout_err ||
        sta->req_err ||
        sta->cpl_stat != 0) {
        pciesys_logerror("rawtlp%d failed:%s%s%s cpl_stat=0x%x\n",
                         port,
                         sta->cpl_data_err ? " cpl_data_err" : "",
                         sta->cpl_timeout_err ? " cpl_timeout_err" : "",
                         sta->req_err ? " req_err" : "",
                         sta->cpl_stat);
        if (sta->cpl_data_err) return -EINVAL;
        if (sta->cpl_timeout_err) return -ETIMEDOUT;
        if (sta->req_err) return -EFAULT;
        if (sta->cpl_stat) return -EIO;
        return -1;
    }
    return 0;
}

static int
hostmem_read_stlp(const int port, const pcie_stlp_t *stlp, void *buf)
{
    rawtlp_req_t req;
    rawtlp_rsp_t rsp;
    sta_itr_raw_tlp_t sta;
    int r;

    memset(&req, 0, sizeof(req));
    int n = pcietlp_encode(stlp, req.b, sizeof(req.b));
    if (n < 0) {
        pciesys_logerror("rawtlp%d encode failed: %s\n",
                         port, pcietlp_get_error());
        return n;
    }

    r = rawtlp_send(port, req.w, n, rsp.w, sizeof(rsp.w), &sta);
    if (r < 0) {
        pciesys_logerror("rawtlp%d send failed: %d\n", port, r);
        return r;
    }

    r = rawtlp_sta_errs(port, &sta);
    if (r < 0) {
        return r;
    }

    /*
     * PCIe transactions are always aligned to dword addresses.
     * The First Byte Enable and Last Byte Enable bits in the
     * transaction select which bytes are valid in the first
     * dword and last dword, respectively.
     * If we're interested in an unaligned start address we
     * adjust the response bytes by the address dword offset.
     */
    const size_t start_offset = stlp->addr & 0x3;
    memcpy(buf, rsp.b + start_offset, stlp->size);

    return stlp->size;
}

static int
hostmem_write_stlp(const int port, const pcie_stlp_t *stlp)
{
    rawtlp_req_t req;
    sta_itr_raw_tlp_t sta;
    int r;

    memset(&req, 0, sizeof(req));
    int n = pcietlp_encode(stlp, req.b, sizeof(req.b));
    if (n < 0) {
        pciesys_logerror("rawtlp%d encode failed: %s\n",
                         port, pcietlp_get_error());
        return n;
    }

    r = rawtlp_send(port, req.w, n, NULL, 0, &sta);
    if (r < 0) {
        pciesys_logerror("rawtlp%d send failed: %d\n", port, r);
        return r;
    }

    r = rawtlp_sta_errs(port, &sta);
    if (r < 0) {
        return r;
    }

    return stlp->size;
}

int
hostmem_read(const int port, const uint16_t reqid,
             uint64_t addr, void *buf, size_t count)
{
    const size_t requested_count = count;
    char *bp = (char *)buf;
    int n;

    pcie_stlp_t stlp;
    memset(&stlp, 0, sizeof(stlp));
    stlp.reqid = reqid;

    /*
     * PCIe reads dword-aligned addresses.  If we're reading
     * an unaligned address and the aligned address/count will
     * span all 8 response dwords, read the unaligned head here.
     */
    while ((addr & 0x3) && (count >= RAWTLP_RSPB - 4)) {
        stlp.addr = addr;
        stlp.type = addr <= 0xffffffff ? PCIE_STLP_MEMRD : PCIE_STLP_MEMRD64;
        stlp.size = MIN(4 - (addr & 0x3), count);

        n = hostmem_read_stlp(port, &stlp, bp);
        if (n < 0) return n;

        bp += n;
        addr += n;
        count -= n;
    }

    /*
     * Raw TLP hardware allows for up to 32 bytes of payload (8 dwords).
     * If we want more than that, break up the requests.
     */
    while (count) {
        stlp.addr = addr;
        stlp.type = addr <= 0xffffffff ? PCIE_STLP_MEMRD : PCIE_STLP_MEMRD64;
        stlp.size = MIN(RAWTLP_RSPB, count);

        n = hostmem_read_stlp(port, &stlp, bp);
        if (n < 0) return n;

        bp += n;
        addr += n;
        count -= n;
    }

    return requested_count;
}

int
hostmem_write(const int port, const uint16_t reqid,
              uint64_t addr, void *buf, size_t count)
{
    const size_t requested_count = count;
    char *bp = (char *)buf;
    int n;

    pcie_stlp_t stlp;
    memset(&stlp, 0, sizeof(stlp));
    stlp.reqid = reqid;

    /*
     * Raw TLP hardware allows for up to 32 bytes of payload (8 dwords),
     * but the stlp encoder only supports 8 bytes of payload at the moment.
     * If we want more than that, break up the requests.
     */
    while (count) {
        stlp.addr = addr;
        stlp.type = addr <= 0xffffffff ? PCIE_STLP_MEMWR : PCIE_STLP_MEMWR64;
        stlp.size = MIN(8, count);
        memcpy(&stlp.data, bp, stlp.size);

        n = hostmem_write_stlp(port, &stlp);
        if (n < 0) return n;

        bp += n;
        addr += n;
        count -= n;
    }

    return requested_count;
}
