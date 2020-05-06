/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#ifndef __RAWTLPPD_H__
#define __RAWTLPPD_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#define RAWTLP_REQDW    12
#define RAWTLP_REQB     (RAWTLP_REQDW * 4)

#define RAWTLP_RSPDW    8
#define RAWTLP_RSPB     (RAWTLP_RSPDW * 4)

typedef union rawtlp_req_u {
    uint32_t w[RAWTLP_REQDW];
    uint8_t  b[RAWTLP_REQB];
} rawtlp_req_t;

typedef union rawtlp_rsp_u {
    uint32_t w[RAWTLP_RSPDW];
    uint8_t  b[RAWTLP_RSPB];
} rawtlp_rsp_t;

typedef union cfg_itr_raw_tlp_cmd_u {
    struct {
        uint32_t dw_cnt:4;
        uint32_t port_id:3;
        uint32_t cmd_go:1;
    } __attribute__((packed));
    uint32_t w;
} cfg_itr_raw_tlp_cmd_t;

typedef union sta_itr_raw_tlp_u {
    struct {
        uint32_t resp_rdy:1;
        uint32_t cpl_stat:3;
        uint32_t cpl_data_err:1;
        uint32_t cpl_timeout_err:1;
        uint32_t req_err:1;
    } __attribute__((packed));
    uint32_t w;
} sta_itr_raw_tlp_t;

int rawtlppd_req(const int port,
                 const uint32_t *reqtlp, const size_t reqtlpsz);
int rawtlppd_resp_rdy(rawtlp_status_t *status);
int rawtlppd_rsp_data(uint32_t *rsptlp, const size_t rsptlpsz);

#ifdef __cplusplus
}
#endif

#endif /* __RAWTLPPD_H__ */
