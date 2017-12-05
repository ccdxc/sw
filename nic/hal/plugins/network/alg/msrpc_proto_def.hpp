#pragma once

#define PDU_REQ             0
#define PDU_RESP            2
#define PDU_REJECT          6
#define PDU_BIND            11
#define PDU_BIND_ACK        12
#define PDU_BIND_NAK        13
#define PDU_ALTER_CTXT      14
#define PDU_ALTER_CTXT_ACK  15

#define PFC_LAST_FRAG  0x02 /* Last fragment */
#define PFC_DG_FRAG    0x04 /* Datagram fragment */

#define EPM_PROTO_UUID 0x0d
#define EPM_PROTO_CN   0x0b
#define EPM_PROTO_DG   0x0a
#define EPM_PROTO_TCP_PORT 0x07
#define EPM_PROTO_UDP_PORT 0x08
#define EPM_PROTO_HOST_ADDR 0x09
#define MAX_FLOORS     5
#define MAX_CONTEXT    256 
#define WORD_BYTES     4
#define SHORT_BYTES    2

namespace hal {
namespace net {

typedef struct uuid_s {
    uint32_t time_lo;
    uint16_t time_mid;
    uint16_t time_hi_vers;
    uint8_t  clock_seq_hi;
    uint8_t  clock_seq_lo;
    uint8_t  node[6];
} uuid_t;

typedef struct msprc_handle_s {
    uint32_t  attr;
    uuid_t    uuid;
} msrpc_handle_t;

typedef struct msrpc_cn_common_hdr_s {
    uint8_t rpc_ver;
    uint8_t rpc_ver_minor;
    uint8_t ptype;
    uint8_t flags;
    uint8_t drep[4];
    uint16_t frag_len;
    uint16_t auth_len;
    uint32_t call_id;
} msrpc_cn_common_hdr_t;

typedef struct msrpc_dg_common_hdr_t {
    uint8_t rpc_ver;
    uint8_t ptype;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t drep[3];
    uint8_t serial_hi;
    uuid_t obj_id;
    uuid_t if_id;
    uuid_t act_id;
    uint32_t server_boot;
    uint32_t if_ver;
    uint32_t seqnum;
    uint16_t opnum;
    uint16_t ihint;
    uint16_t ahint;
    uint16_t frag_len;
    uint16_t frag_num;
    uint8_t auth_proto;
    uint8_t serial_lo;
} msrpc_dg_common_hdr_t;

typedef struct port_any_s {
    uint16_t len;
    char *port_spec;
} port_any_t;

typedef struct p_syntax_id_ {
    uuid_t   if_uuid;
    uint32_t if_vers;
} p_syntax_id_t;

typedef struct p_cont_elem_ {
    uint16_t        context_id;
    uint8_t         num_xfer_syn;
    uint8_t         reserved;
    p_syntax_id_t   abs_syntax;
    p_syntax_id_t   xfer_syn;
} p_cont_elem_t;

typedef struct p_cont_list_ {
    uint8_t       num_elm;
    uint8_t       rsvd;
    uint16_t      rsvd2;
    p_cont_elem_t *cont_elem;
} p_cont_list_t;

typedef struct p_result_ {
    uint8_t        result;
    uint8_t        fail_reason;
    p_syntax_id_t  xfer_syn;
} p_result_t;

typedef struct p_result_list_s {
    uint8_t      num_rslts;
    uint8_t      rsvd;
    uint16_t     rsvd2;
    p_result_t   *rslts;
} p_result_list_t;

typedef struct msrpc_bind_hdr_s {
    uint16_t       max_xmit_frag;
    uint16_t       max_recv_frag;
    uint32_t       assoc_group_id;
    p_cont_list_t  context_list;
} msrpc_bind_hdr_t;

typedef struct msrpc_bind_ack_hdr_s {
    uint16_t        max_xmit_frag;
    uint16_t        max_recv_frag;
    uint32_t        assoc_group_id;
    port_any_t      sec_addr; 
    p_result_list_t rlist;
} msrpc_bind_ack_hdr_t;

typedef struct msrpc_req_hdr_s {
    uint32_t   alloc_hint;
    uint16_t   ctxt_id;
    uint16_t   opnum;
    uint32_t   uuid_ptr;
    uuid_t     uuid;
} msrpc_req_hdr_t;

typedef struct msrpc_rsp_hdr_s {
    uint32_t   alloc_hint;
    uint16_t   ctxt_id;
    uint8_t    cancel_cnt;
    uint8_t    rsvd;
} msrpc_rsp_hdr_t;

typedef struct msrpc_epm_flr_s {
    uint16_t     lhs_length;
    uint8_t      protocol;
    uuid_t       uuid;
    uint16_t     version;
    uint16_t     rhs_length;
    uint16_t     minor_vers;
    uint16_t     port;
    ipvx_addr_t  ip;
} msrpc_epm_flr_t;

typedef struct msrcp_twr_p_s {
    uint32_t         twr_arr_len;
    uint16_t         num_floors;
    msrpc_epm_flr_t flrs[MAX_FLOORS];
} msrpc_twr_p_t;

typedef struct msrpc_map_twr_s {
    uint32_t       twr_ptr;
    uint32_t       twr_lgth;
    msrpc_twr_p_t  twr_arr;
} msrpc_map_twr_t;

typedef struct msrpc_epm_req_hdr_s {
    msrpc_map_twr_t twr;
    msrpc_handle_t  hdl;
    uint32_t        max_twrs;
} msrpc_epm_req_hdr_t;

typedef struct msrpc_epm_rsp_hdr_s {
    msrpc_handle_t   hdl;
    uint32_t         num_twrs;
    uint32_t         max_cnt;
    uint32_t         offset;
    uint32_t         actual_cnt;
    msrpc_map_twr_t  twr;
} msrpc_epm_rsp_hdr_t;

}
}
