/*
 * Copyright (c) 2020, Pensando Systems Inc.
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

#include "platform/pal/include/pal.h"
#include "platform/pcieport/include/pcieport.h"
#include "platform/pcieport/include/rawtlp.h"
#include "pcieportpd.h"
#include "rawtlppd.h"

int
rawtlppd_req(const int port, const uint32_t *reqtlp, const size_t reqtlpsz)
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
rawtlppd_sta_itr_raw_tlp(sta_itr_raw_tlp_t *sta)
{
    sta->w = pal_reg_rd32(PXB_(STA_ITR_RAW_TLP));
}

int
rawtlppd_resp_rdy(rawtlp_status_t *status)
{
    sta_itr_raw_tlp_t stabuf, *sta = &stabuf;
    rawtlppd_sta_itr_raw_tlp(sta);
    memcpy(status, sta, sizeof(*sta));
#ifdef __aarch64__
    return sta->resp_rdy;
#else
    return 1;
#endif
}

int
rawtlppd_rsp_data(uint32_t *rsptlp, const size_t rsptlpsz)
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
