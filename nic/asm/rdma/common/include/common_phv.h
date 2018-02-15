#ifndef __COMMON_PHV_H
#define __COMMON_PHV_H

#include "types.h"

#define S2S_DATA_WIDTH 160
#define GLOBAL_DATA_WIDTH 128
#define TO_STAGE_DATA_WIDTH 128

struct p4plus_to_p4_header_t {
    p4plus_app_id      : 4;
    pad                : 4;
    flags              : 8;
    udp_opt_bytes      : 8;  // exclude these bytes from udp payload_len
    rsvd               : 24;
    ip_id_delta        : 16;
    tcp_seq_delta      : 32;
    gso_start           : 14;
    gso_offset          : 14;
    byte_align_pad      : 3;
    gso_valid           : 1;
    vlan_tag           : 16;
};

#define P4PLUS_TO_P4_APP_ID     p.p4plus_to_p4.p4plus_app_id
#define P4PLUS_TO_P4_FLAGS      p.p4plus_to_p4.flags
#define P4PLUS_TO_P4_VLAN_TAG   p.p4plus_to_p4.vlan_tag
#define P4PLUS_TO_P4_UDP_OPT_BYTES p.p4plus_to_p4.udp_opt_bytes

struct phv_to_stage_t {
    pad: TO_STAGE_DATA_WIDTH;
};

#define PT_BASE_ADDR_GET(_r) \
    sll     _r, k.global.pt_base_addr_page_id, HBM_PAGE_SIZE_SHIFT;

#define KT_BASE_ADDR_GET(_r, _tmp_r) \
    add    _tmp_r, CAPRI_LOG_SIZEOF_U64, k.global.log_num_pt_entries; \
    sllv   _tmp_r, 1, _tmp_r; \
    add    _r, _tmp_r, k.global.pt_base_addr_page_id, HBM_PAGE_SIZE_SHIFT;

#define CQCB_BASE_ADDR_GET(_r, _cqcb_page_id) \
    sll     _r, _cqcb_page_id, HBM_PAGE_SIZE_SHIFT;

#define CQCB_ADDR_GET(_r, _cqid, _cqcb_page_id) \
    CQCB_BASE_ADDR_GET(_r, _cqcb_page_id);\
    add _r, _r, _cqid, LOG_SIZEOF_CQCB_T

#define EQCB_BASE_ADDR_GET(_r, _tmp_r, _cqcb_page_id, _log_cq_entries) \
    add    _tmp_r, _log_cq_entries, LOG_SIZEOF_CQCB_T; \
    sllv   _tmp_r, 1, _tmp_r; \
    add   _r, _tmp_r, _cqcb_page_id, HBM_PAGE_SIZE_SHIFT; 

#define EQCB_ADDR_GET(_r, _tmp_r, _eqid, _cqcb_page_id, _log_cq_entries) \
    EQCB_BASE_ADDR_GET(_r, _tmp_r, _cqcb_page_id, _log_cq_entries); \
    add _r, _r, _eqid, LOG_SIZEOF_EQCB_T

#define PHV_GLOBAL_COMMON_T struct phv_global_common_t
struct phv_global_common_t {
    lif: 11;
    qid: 24;
    qtype: 3;
    cb_addr: 25;
    pt_base_addr_page_id: 22;
    log_num_pt_entries: 5;
    pad: 22;
    union roce_opcode_flags_t flags;
    //prefetch_pool_base_addr_page_id: 20;
    //log_num_prefetch_pool_entries: 5;
};

#endif //__COMMON_PHV_H
