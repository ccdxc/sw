#ifndef __COMMON_PHV_H
#define __COMMON_PHV_H

#include "types.h"
#include "capri.h"

#define S2S_DATA_WIDTH 160
#define GLOBAL_DATA_WIDTH 128
#define TO_STAGE_DATA_WIDTH 128

struct p4plus_to_p4_header_t {
    p4plus_app_id      : 4;
    pad                : 4;
    flow_index         : 24;
    flags              : 8;
    udp_opt_bytes      : 8;  // exclude these bytes from udp payload_len
    dst_lport          : 11;
    dst_lport_valid    : 1;
    pad1               : 4;
    ip_id_delta        : 16;
    tcp_seq_delta      : 32;
    gso_start           : 14;
    byte_align_pad0     : 2;
    gso_offset          : 14;
    flow_index_valid    : 1;
    gso_valid           : 1;
    vlan_tag           : 16;
};

#define P4PLUS_TO_P4_APP_ID     p.p4plus_to_p4.p4plus_app_id
#define P4PLUS_TO_P4_FLAGS      p.p4plus_to_p4.flags
#define P4PLUS_TO_P4_VLAN_TAG   p.p4plus_to_p4.vlan_tag
#define P4PLUS_TO_P4_UDP_OPT_BYTES p.p4plus_to_p4.udp_opt_bytes
#define P4PLUS_TO_P4_FLOW_INDEX p.p4plus_to_p4.flow_index
#define P4PLUS_TO_P4_FLOW_INDEX_VALID p.p4plus_to_p4.flow_index_valid

struct phv_to_stage_t {
    pad: TO_STAGE_DATA_WIDTH;
};

#define PT_BASE_ADDR_GET(_r) \
    sll     _r, k.global.pt_base_addr_page_id, HBM_PAGE_SIZE_SHIFT;

#define PT_BASE_ADDR_GET2(_r) \
    sll     _r, k.{phv_global_common_pt_base_addr_page_id_sbit0_ebit0...phv_global_common_pt_base_addr_page_id_sbit17_ebit21}, HBM_PAGE_SIZE_SHIFT;

#define PT_BASE_ADDR_GET3(_r)                                           \
    sll     _r, k.{phv_global_common_pt_base_addr_page_id_sbit0_ebit5...phv_global_common_pt_base_addr_page_id_sbit6_ebit21}, HBM_PAGE_SIZE_SHIFT;

#define KT_BASE_ADDR_GET(_r, _tmp_r)                                  \
    add    _tmp_r, CAPRI_LOG_SIZEOF_U64, k.global.log_num_pt_entries; \
    sllv   _tmp_r, 1, _tmp_r; \
    add    _r, _tmp_r, k.global.pt_base_addr_page_id, HBM_PAGE_SIZE_SHIFT;

#define KT_BASE_ADDR_GET2(_r, _tmp_r) \
    add    _tmp_r, CAPRI_LOG_SIZEOF_U64, k.{phv_global_common_log_num_pt_entries_sbit0_ebit2...phv_global_common_log_num_pt_entries_sbit3_ebit4}; \
    sllv   _tmp_r, 1, _tmp_r; \
    add    _r, _tmp_r, CAPRI_KEY_RANGE(phv_global_common_pt_base_addr_page_id,sbit0_ebit0, sbit17_ebit21), HBM_PAGE_SIZE_SHIFT;

#define CB_BASE_ADDR_GET(_r, _cqcb_base_addr_hi) \
    sll     _r, _cqcb_base_addr_hi, CQCB_ADDR_HI_SHIFT;

#define CQCB_ADDR_GET(_r, _cqid, _cqcb_base_addr_hi) \
    CB_BASE_ADDR_GET(_r, _cqcb_base_addr_hi);\
    add _r, _r, _cqid, LOG_SIZEOF_CQCB_T

#define SQCB_ADDR_GET(_r, _qid, _cb_page_id) \
    CB_BASE_ADDR_GET(_r, _cb_page_id);        \
    add _r, _r, _qid, LOG_SIZEOF_SQCB_T

#define RQCB_ADDR_GET(_r, _qid, _cb_page_id) \
    CB_BASE_ADDR_GET(_r, _cb_page_id);        \
    add _r, _r, _qid, LOG_SIZEOF_RQCB_T

#define EQCB_BASE_ADDR_GET(_r, _tmp_r, _cqcb_base_addr_hi, _log_cq_entries) \
    add    _tmp_r, _log_cq_entries, LOG_SIZEOF_CQCB_T; \
    sllv   _tmp_r, 1, _tmp_r; \
    add   _r, _tmp_r, _cqcb_base_addr_hi, CQCB_ADDR_HI_SHIFT; 

#define EQCB_ADDR_GET(_r, _tmp_r, _eqid, _cqcb_base_addr_hi, _log_cq_entries) \
    EQCB_BASE_ADDR_GET(_r, _tmp_r, _cqcb_base_addr_hi, _log_cq_entries); \
    add _r, _r, _eqid, LOG_SIZEOF_EQCB_T

#define PHV_GLOBAL_COMMON_T struct phv_global_common_t
struct phv_global_common_t {
    lif: 11;
    qid: 24;
    qtype: 3;
    cb_addr: 25;
    pt_base_addr_page_id: 22;
    log_num_pt_entries: 5;
    union phv_global_pad_t pad;
    union roce_opcode_flags_t flags;
};

#define K_GLOBAL_LIF CAPRI_KEY_RANGE(phv_global_common, lif_sbit0_ebit7, lif_sbit8_ebit10)
#define K_GLOBAL_QID CAPRI_KEY_RANGE(phv_global_common, qid_sbit0_ebit4, qid_sbit21_ebit23)
#define K_GLOBAL_QTYPE CAPRI_KEY_FIELD(phv_global_common, qtype)
#define K_GLOBAL_FLAGS k.{common_global_global_data_sbit112_ebit119...common_global_global_data_sbit120_ebit127}

// Req_tx
#define K_GLOBAL_SPEC_CINDEX CAPRI_KEY_RANGE(phv_global_common, spec_cindex_sbit0_ebit5, spec_cindex_sbit14_ebit15)

// Resp_rx
#define K_GLOBAL_LOG_NUM_KT_ENTRIES CAPRI_KEY_FIELD(phv_global_common, log_num_kt_entries)

#define K_GLOBAL_FLAG(_f) CAPRI_KEY_FIELD(phv_global_common, _f)

#endif //__COMMON_PHV_H
