#ifndef __APP_REDIR_HEADERS_H__
#define __APP_REDIR_HEADERS_H__

#define BITS_PER_BYTE                   8
#define SIZE_IN_BITS(bytes)             ((bytes) * BITS_PER_BYTE)
#define SIZE_IN_BYTES(bits)             ((bits) / BITS_PER_BYTE)

/*
 * Meta header starts with a standard struct ethhdr
 */
#define ETH_ALEN                        6
#define ETH_PLEN                        2
#define PEN_APP_REDIR_ETHERTYPE         0xabcd  // temporary

struct pen_app_redir_header_t {
    h_dest              : 48;
    h_source            : 48;
    h_proto             : 16;
};

#define PEN_APP_REDIR_HEADER_SIZE                                               \
        SIZE_IN_BYTES(sizeof(struct pen_app_redir_header_t))

/*
 * Version and format specific meta header portion
 */
#define PEN_RAW_REDIR_V1_FORMAT         0
#define PEN_PROXY_REDIR_V1_FORMAT       1

struct pen_app_redir_version_header_t {
    hdr_len             : 16;
    format              : 8;
    end_pad             : 8;
};

#define PEN_APP_REDIR_VERSION_HEADER_SIZE                                       \
        SIZE_IN_BYTES(sizeof(struct pen_app_redir_version_header_t))

/*
 * Version 1 raw meta header:
 * Total sizeof(pen_app_redir_header_t + pen_app_redir_version_header_t +
 *  pen_raw_redir_header_v1_t) must equal P4PLUS_RAW_REDIR_HDR_SZ (40 bytes)
 */
struct pen_raw_redir_header_v1_t {
    flow_id             : 32;
    flags               : 16;
    vrf                 : 16;
    redir_miss_pkt_p    : 64;
    end_pad             : 48;
};

#define PEN_RAW_REDIR_HEADER_V1_SIZE                                            \
        SIZE_IN_BYTES(sizeof(struct pen_raw_redir_header_v1_t))

/*
 * Meta header flags
 */
#define PEN_APP_REDIR_L3_CSUM_CHECKED       0x0001
#define PEN_APP_REDIR_L4_CSUM_CHECKED       0x0002
#define PEN_APP_REDIR_SPAN_INSTANCE         0x0004
#define PEN_APP_REDIR_PIPELINE_LOOPBK_EN    0x0008
#define PEN_APP_REDIR_A0_RNMPR_SMALL        0x0010      // deprecated
#define PEN_APP_REDIR_A1_RNMPR_SMALL        0x0020      // deprecated
#define PEN_APP_REDIR_A2_RNMPR_SMALL        0x0040      // deprecated
#define PEN_APP_REDIR_DONT_FREE_DESC        0x0080


/*
 * Version 1 Proxy meta header
 */
struct pen_proxy_redir_header_v1_t {
    flow_id             : 32;
    flags               : 16;
    vrf                 : 16;
    ip_sa               : 128;
    ip_da               : 128;
    sport               : 16;
    dport               : 16;
    af                  : 8;
    ip_proto            : 8;
    tcp_flags           : 8;
    end_pad             : 8;
};

#define PEN_PROXY_REDIR_HEADER_V1_SIZE                                          \
        SIZE_IN_BYTES(sizeof(struct pen_proxy_redir_header_v1_t))


#endif      // __APP_REDIR_HEADERS_H__

