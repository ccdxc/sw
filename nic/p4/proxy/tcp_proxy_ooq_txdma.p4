/*************************************************************/
/* tcp_proxy_ooq_txdma.p4
/************************************************************/


#include "../common-p4+/common_txdma_dummy.p4"
#include "../../include/capri_common.h"

#define tx_table_s0_t0 s0_t0_ooq_tcp_tx
#define tx_table_s1_t0 s1_t0_ooq_tcp_tx
#define tx_table_s2_t0 s2_t0_ooq_tcp_tx
#define tx_table_s3_t0 s3_t0_ooq_tcp_tx
#define tx_table_s3_t1 s3_t1_ooq_tcp_tx
#define tx_table_s4_t0 s4_t0_ooq_tcp_tx

#define tx_table_s0_t0_action ooq_tcp_txdma_load_stage0
#define tx_table_s1_t0_action ooq_tcp_txdma_load_rx2tx_slot
#define tx_table_s2_t0_action ooq_tcp_txdma_load_qbase_addr
#define tx_table_s3_t0_action ooq_tcp_txdma_load_one_descr
#define tx_table_s4_t0_action ooq_tcp_txdma_generate_dummy_pkt

#include "../common-p4+/common_txdma.p4"


header_type ooq_tcp_txdma_qstate_d_t {
    fields {
        CAPRI_QSTATE_HEADER_COMMON
        CAPRI_QSTATE_HEADER_RING(0)
        ooq_per_flow_ring_base     : 64;
        current_descr_qbase_addr   : 64;
        ooq_per_flow_ring_entries  : 8;
        num_entries                : 8;
        num_pkts                   : 16;
        curr_index                 : 16;
        ooq_proc_in_progress       : 1;
        ooq_proc_flags             : 7;
    }
}

header_type ooq_tcp_txdma_load_rx2tx_slot_d_t {
    fields {
        ooq_qbase_addr      : 64;
        num_entries         : 16;
        pad                 : 48;
    }
}

header_type ooq_tcp_txdma_load_qbase_addr_d_t {
    fields {
        ooq_ring_base_addr  : 64;
    }
}

header_type ooq_tcp_txdma_load_one_descr_d_t {
    fields {
        Addr0      : 64;
        Offset0    : 32;
        Length0    : 32;
        Addr1      : 64;
        Offset1    : 32;
        Length1    : 32;
        Addr2      : 64;
        Offset2    : 32;
        Length2    : 32;
    }
}

header_type ooq_tcp_txdma_load_tcp_header_in_descr_d_t {
    fields {
        tcp_app_header_p4plus_app_id : 4; 
        tcp_app_header_table0_valid : 1; 
        tcp_app_header_table1_valid : 1; 
        tcp_app_header_table2_valid : 1; 
        tcp_app_header_table3_valid : 1; 
        tcp_app_header_from_ooq_txdma : 1; 
        tcp_app_header_num_sack_blocks : 7; 
        tcp_app_header_payload_len : 16; 
        tcp_app_header_srcPort : 16; 
        tcp_app_header_dstPort : 16; 
        tcp_app_header_seqNo : 32; 
        tcp_app_header_ackNo : 32; 
        tcp_app_header_dataOffset : 4; 
        tcp_app_header_res : 4; 
        tcp_app_header_flags : 8; 
        tcp_app_header_window : 16; 
        tcp_app_header_urgentPtr : 16; 
        tcp_app_header_ts : 32; 
        tcp_app_header_prev_echo_ts : 32;
        pad : 16; 
    }
}

header_type common_global_phv_t {
    fields {
        // global k (max 128)
        fid                     : 24;
        qstate_addr             : 34;
        pad                     : 70;
    }
}

header_type to_stage_1_phv_t {
    fields {
        pad   : 128;
    }
}

header_type to_stage_2_phv_t {
    fields {
        one_descr_addr : 64;
        pad   : 64;
    }
}

header_type to_stage_3_phv_t {
    fields {
        pad   : 128;
    }
}

header_type to_stage_4_phv_t {
    fields {
        new_qbase_addr : 64;
        num_entries    : 16;
        curr_index     : 16;
        pad            : 32;
    }
}

header_type to_stage_5_phv_t {
    fields {
        pad   : 128;
    }
}

#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.fid, common_phv.fid); \
    modify_field(common_global_scratch.qstate_addr, common_phv.qstate_addr); \
    modify_field(common_global_scratch.pad, common_phv.pad);
  
@pragma scratch_metadata
metadata ooq_tcp_txdma_qstate_d_t ooq_tcp_txdma_qstate_d;
@pragma scratch_metadata
metadata ooq_tcp_txdma_load_rx2tx_slot_d_t ooq_tcp_txdma_load_rx2tx_slot_d;
@pragma scratch_metadata
metadata ooq_tcp_txdma_load_qbase_addr_d_t ooq_tcp_txdma_load_qbase_addr_d;
@pragma scratch_metadata
metadata ooq_tcp_txdma_load_one_descr_d_t ooq_tcp_txdma_load_one_descr_d;
@pragma scratch_metadata
metadata ooq_tcp_txdma_load_tcp_header_in_descr_d_t ooq_tcp_txdma_load_tcp_header_in_descr_d;

@pragma scratch_metadata
metadata to_stage_1_phv_t to_s1_scratch;
@pragma scratch_metadata
metadata to_stage_2_phv_t to_s2_scratch;
@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;
@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;
@pragma scratch_metadata
metadata to_stage_5_phv_t to_s5_scratch;



@pragma pa_header_union ingress to_stage_1
metadata to_stage_1_phv_t to_s1;
@pragma pa_header_union ingress to_stage_2
metadata to_stage_2_phv_t to_s2;
@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;
@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;
@pragma pa_header_union ingress to_stage_5
metadata to_stage_5_phv_t to_s5;

@pragma dont_trim
@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_header_t tcp_app_header;
@pragma pa_header_union ingress common_global
metadata common_global_phv_t common_phv;
@pragma scratch_metadata
metadata common_global_phv_t common_global_scratch;


@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2pkt_t intrinsic_dma;    // dma cmd 1
@pragma dont_trim
metadata dma_cmd_mem2pkt_t l2l3_header_dma;  // dma cmd 2
@pragma dont_trim
metadata dma_cmd_phv2pkt_t tcp_header_dma;   // dma cmd 3


// Stage-0 Table-0
action ooq_tcp_txdma_load_stage0(rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0,ci_0, 
                                 current_descr_qbase_addr, num_entries, num_pkts, curr_index, ooq_proc_in_progress,
                                 ooq_proc_flags, ooq_per_flow_ring_base)
{
    modify_field(ooq_tcp_txdma_qstate_d.rsvd, rsvd);                                                                  
    modify_field(ooq_tcp_txdma_qstate_d.cosA, cosA);                                                                 
    modify_field(ooq_tcp_txdma_qstate_d.cosB, cosB);                                                               
    modify_field(ooq_tcp_txdma_qstate_d.cos_sel, cos_sel);                                                       
    modify_field(ooq_tcp_txdma_qstate_d.eval_last, eval_last);                                                 
    modify_field(ooq_tcp_txdma_qstate_d.host, host);                                                         
    modify_field(ooq_tcp_txdma_qstate_d.total, total);                                                    
    modify_field(ooq_tcp_txdma_qstate_d.pid, pid);                                                      
    modify_field(ooq_tcp_txdma_qstate_d.pi_0, pi_0);                                                
    modify_field(ooq_tcp_txdma_qstate_d.ci_0, ci_0);     
    modify_field(ooq_tcp_txdma_qstate_d.current_descr_qbase_addr, current_descr_qbase_addr);
    modify_field(ooq_tcp_txdma_qstate_d.num_entries, num_entries);
    modify_field(ooq_tcp_txdma_qstate_d.num_pkts, num_pkts);
    modify_field(ooq_tcp_txdma_qstate_d.curr_index, curr_index);
    modify_field(ooq_tcp_txdma_qstate_d.ooq_proc_in_progress, ooq_proc_in_progress);
    modify_field(ooq_tcp_txdma_qstate_d.ooq_proc_flags, ooq_proc_flags);
    modify_field(ooq_tcp_txdma_qstate_d.ooq_per_flow_ring_base, ooq_per_flow_ring_base);
 
}

// Stage-1 Table-0
action ooq_tcp_txdma_load_rx2tx_slot(ooq_qbase_addr, num_entries, pad)
{
    GENERATE_GLOBAL_K
    modify_field(ooq_tcp_txdma_load_rx2tx_slot_d.ooq_qbase_addr, ooq_qbase_addr);
    modify_field(ooq_tcp_txdma_load_rx2tx_slot_d.num_entries, num_entries);
    modify_field(ooq_tcp_txdma_load_rx2tx_slot_d.pad, pad);
}

// Stage-2 Table-0
action ooq_tcp_txdma_load_qbase_addr(ooq_ring_base_addr)
{
    GENERATE_GLOBAL_K
    modify_field(ooq_tcp_txdma_load_qbase_addr_d.ooq_ring_base_addr, ooq_ring_base_addr);

    modify_field(to_s2_scratch.one_descr_addr, to_s2.one_descr_addr);
}

// Stage-3 Table-0
action ooq_tcp_txdma_load_one_descr(tcp_app_header_p4plus_app_id,
                                              tcp_app_header_table0_valid, tcp_app_header_table1_valid, 
                                              tcp_app_header_table2_valid, tcp_app_header_table3_valid, 
                                              tcp_app_header_from_ooq_txdma, tcp_app_header_num_sack_blocks, 
                                              tcp_app_header_payload_len, tcp_app_header_srcPort, 
                                              tcp_app_header_dstPort, tcp_app_header_seqNo, 
                                              tcp_app_header_ackNo, tcp_app_header_dataOffset, 
                                              tcp_app_header_res, tcp_app_header_flags, 
                                              tcp_app_header_window, tcp_app_header_urgentPtr, 
                                              tcp_app_header_ts, tcp_app_header_prev_echo_ts, pad) 
{
    GENERATE_GLOBAL_K
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_p4plus_app_id, tcp_app_header_p4plus_app_id);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_table0_valid, tcp_app_header_table0_valid);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_table1_valid, tcp_app_header_table1_valid);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_table2_valid, tcp_app_header_table2_valid);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_table3_valid, tcp_app_header_table3_valid);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_from_ooq_txdma, tcp_app_header_from_ooq_txdma);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_num_sack_blocks, tcp_app_header_num_sack_blocks);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_payload_len, tcp_app_header_payload_len);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_srcPort, tcp_app_header_srcPort);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_dstPort, tcp_app_header_dstPort);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_seqNo, tcp_app_header_seqNo);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_ackNo, tcp_app_header_ackNo);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_dataOffset, tcp_app_header_dataOffset);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_res, tcp_app_header_res);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_flags, tcp_app_header_flags);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_window, tcp_app_header_window);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_urgentPtr, tcp_app_header_urgentPtr);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_ts, tcp_app_header_ts);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.tcp_app_header_prev_echo_ts, tcp_app_header_prev_echo_ts);
    modify_field(ooq_tcp_txdma_load_tcp_header_in_descr_d.pad, pad);
}

// Stage-4 Table-0
action ooq_tcp_txdma_generate_dummy_pkt(rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0,ci_0, 
                                 current_descr_qbase_addr, num_entries, num_pkts, curr_index, ooq_proc_in_progress,
                                 ooq_proc_flags, ooq_per_flow_ring_base)
{
    GENERATE_GLOBAL_K
    modify_field(to_s4.new_qbase_addr, to_s4_scratch.new_qbase_addr);
    modify_field(to_s4.num_entries, to_s4_scratch.num_entries);
    modify_field(to_s4.pad, to_s4_scratch.pad);
    modify_field(ooq_tcp_txdma_qstate_d.rsvd, rsvd);                                                                  
    modify_field(ooq_tcp_txdma_qstate_d.cosA, cosA);                                                                 
    modify_field(ooq_tcp_txdma_qstate_d.cosB, cosB);                                                               
    modify_field(ooq_tcp_txdma_qstate_d.cos_sel, cos_sel);                                                       
    modify_field(ooq_tcp_txdma_qstate_d.eval_last, eval_last);                                                 
    modify_field(ooq_tcp_txdma_qstate_d.host, host);                                                         
    modify_field(ooq_tcp_txdma_qstate_d.total, total);                                                    
    modify_field(ooq_tcp_txdma_qstate_d.pid, pid);                                                      
    modify_field(ooq_tcp_txdma_qstate_d.pi_0, pi_0);                                                
    modify_field(ooq_tcp_txdma_qstate_d.ci_0, ci_0);     
    modify_field(ooq_tcp_txdma_qstate_d.current_descr_qbase_addr, current_descr_qbase_addr);
    modify_field(ooq_tcp_txdma_qstate_d.num_entries, num_entries);
    modify_field(ooq_tcp_txdma_qstate_d.num_pkts, num_pkts);
    modify_field(ooq_tcp_txdma_qstate_d.curr_index, curr_index);
    modify_field(ooq_tcp_txdma_qstate_d.ooq_proc_in_progress, ooq_proc_in_progress);
    modify_field(ooq_tcp_txdma_qstate_d.ooq_proc_flags, ooq_proc_flags);
}

