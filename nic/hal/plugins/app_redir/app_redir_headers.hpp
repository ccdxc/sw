// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __APP_REDIR_HEADERS_H__
#define __APP_REDIR_HEADERS_H__

/*
 * Meta header starts with a standard struct ethhdr
 */
#define PEN_APP_REDIR_ETHERTYPE         0xabcd  // temporary

#ifndef ETH_ADDR_LEN
#define ETH_ADDR_LEN                    6
#endif

typedef struct pen_app_redir_header_s {
    uint8_t             h_dest[ETH_ADDR_LEN];
    uint8_t             h_source[ETH_ADDR_LEN];
    uint16_t            h_proto;
} __attribute__ ((__packed__)) pen_app_redir_header_t;

#define PEN_APP_REDIR_HEADER_SIZE       sizeof(pen_app_redir_header_t)

/*
 * Version and format specific meta header portion
 */
#define PEN_RAW_REDIR_V1_FORMAT         0
#define PEN_PROXY_REDIR_V1_FORMAT       1

typedef struct pen_app_redir_version_header_s {
    uint16_t            hdr_len;
    uint8_t             format;
    uint8_t             end_pad;
} __attribute__ ((__packed__)) pen_app_redir_version_header_t;

#define PEN_APP_REDIR_VERSION_HEADER_SIZE   sizeof(pen_app_redir_version_header_t)

/*
 * Version 1 raw meta header:
 * Total sizeof(pen_app_redir_header_t + pen_app_redir_version_header_t +
 *  pen_raw_redir_header_v1_t) must equal P4PLUS_RAW_REDIR_HDR_SZ (40 bytes)
 */
typedef struct pen_raw_redir_header_v1_s {
    uint32_t            flow_id;
    uint16_t            flags;
    uint16_t            vrf;
    uint64_t            redir_miss_pkt_p;
    uint8_t             end_pad[6];
} __attribute__ ((__packed__)) pen_raw_redir_header_v1_t;

#define PEN_RAW_REDIR_HEADER_V1_SIZE    sizeof(pen_raw_redir_header_v1_t)

/*
 * Meta header flags
 */
#define PEN_APP_REDIR_L3_CSUM_CHECKED       0x0001
#define PEN_APP_REDIR_L4_CSUM_CHECKED       0x0002
#define PEN_APP_REDIR_SPAN_INSTANCE         0x0004
#define PEN_APP_REDIR_PIPELINE_LOOPBK_EN    0x0008
#define PEN_APP_REDIR_A0_RNMPR_SMALL        0x0010
#define PEN_APP_REDIR_A1_RNMPR_SMALL        0x0020
#define PEN_APP_REDIR_A2_RNMPR_SMALL        0x0040


/*
 * Version 1 Proxy meta header
 */
typedef struct pen_proxy_redir_header_v1_s {
    uint32_t            flow_id;
    uint16_t            flags;

    /*
     * The following static flow key fields (from vrf to ip_proto)
     * must be sized and ordered exactly as defined in the structure
     * proxyrcb_flow_key_t in p4/app-redir-p4+/proxyr_txdma.p4
     */
    uint16_t            vrf;
    uint32_t            ip_sa[4];
    uint32_t            ip_da[4];
    uint16_t            sport;
    uint16_t            dport;
    uint8_t             af;
    uint8_t             ip_proto;
    uint8_t             tcp_flags;
    uint8_t             end_pad;
} __attribute__ ((__packed__)) pen_proxy_redir_header_v1_t;

#define PEN_PROXY_REDIR_HEADER_V1_SIZE  sizeof(pen_proxy_redir_header_v1_t)

/*
 * Fully assembled meta headers
 */
typedef struct pen_app_redir_header_v1_full_s {
    pen_app_redir_header_t          app;
    pen_app_redir_version_header_t  ver;
    union {
        pen_raw_redir_header_v1_t   raw;
        pen_proxy_redir_header_v1_t proxy;
    };
} __attribute__ ((__packed__)) pen_app_redir_header_v1_full_t;


#define PEN_APP_REDIR_HEADER_MIN_SIZE       \
    (PEN_APP_REDIR_HEADER_SIZE + PEN_APP_REDIR_VERSION_HEADER_SIZE)
#define PEN_RAW_REDIR_HEADER_V1_FULL_SIZE   \
    (PEN_APP_REDIR_HEADER_MIN_SIZE + PEN_RAW_REDIR_HEADER_V1_SIZE)
#define PEN_PROXY_REDIR_HEADER_V1_FULL_SIZE  \
    (PEN_APP_REDIR_HEADER_MIN_SIZE + PEN_PROXY_REDIR_HEADER_V1_SIZE)
    
#endif      // __APP_REDIR_HEADERS_H__
