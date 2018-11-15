#include "../common-p4+/common_txdma_dummy.p4"
#include "cpu_rxtx_common.p4"

/*******************************************************
 * Table and actions
 ******************************************************/
#define tx_table_s0_t0 cpu_tx_initial_action
#define tx_table_s0_t0_action cpu_tx_initial_action

#define tx_table_s1_t0 cpu_tx_read_asq_ci
#define tx_table_s1_t0_action read_asq_ci

#define tx_table_s2_t0 cpu_tx_read_asq_descr
#define tx_table_s2_t0_action read_asq_descr

#define tx_table_s3_t0 cpu_tx_read_cpu_hdr
#define tx_table_s3_t0_action read_cpu_hdr

#define tx_table_s3_t1 cpu_tx_read_l2_vlan_hdr
#define tx_table_s3_t1_action read_l2_vlan_hdr

#define tx_table_s4_t0 cpu_tx_write_pkt
#define tx_table_s4_t0_action write_pkt

#define tx_table_s5_t0_action cpu_tx_sem_full_drop

#include "../common-p4+/common_txdma.p4"

#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.write_vlan_tag, common_phv.write_vlan_tag); \
    modify_field(common_global_scratch.rem_vlan_tag, common_phv.rem_vlan_tag); \
    modify_field(common_global_scratch.add_qs_trlr, common_phv.add_qs_trlr); \
    modify_field(common_global_scratch.free_buffer, common_phv.free_buffer); \
    modify_field(common_global_scratch.ascq_base, common_phv.ascq_base); \
    modify_field(common_global_scratch.cpucb_addr, common_phv.cpucb_addr); \
   

/********************
 * Packet Headers
 *******************/
header_type vlan_hdr_t {
    fields {
        etherType               : 16;
        pcp                     : 3;
        dei                     : 1;
        vid                     : 12;
    }    
}

/******************************************************************************
 * D-vectors
 *****************************************************************************/
header_type cpu_txdma_initial_action_t {
    fields {
        // 8 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        
        CAPRI_QSTATE_HEADER_RING(0)

        asq_base                : 64;
        ascq_base               : 64;
        ascq_sem_inf_addr       : 64;
        asq_pi_ci_eq_drops      : 32;
        asq_total_pkts          : 64;
        ascq_sem_full_drops     : 64;
        ascq_free_requests      : 64;
    }
}

header_type cpu_txdma_initial_action_with_pc_t {
    fields {
        pc                       : 8;
        // 8 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        
        CAPRI_QSTATE_HEADER_RING(0)

        asq_base                : 64;
        ascq_base               : 64;
        ascq_sem_inf_addr       : 64;
        asq_pi_ci_eq_drops      : 32;
        asq_total_pkts          : 64;
        ascq_sem_full_drops     : 32;
        ascq_free_requests      : 32;
    }
}

// d for stage1
header_type read_asq_ci_d_t {
    fields {
        desc_addr      : 64;     
    }    
}

// d fpr stage2
// use pkt_descr_t

// d for stage 4
header_type write_pkt_d_t {
    fields {
        ascq_pindex      : 32;
        ascq_full        : 8;
    }
} 

/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/
header_type common_global_phv_t {
    fields {
        // global k (max 128)
        write_vlan_tag          : 1;
        rem_vlan_tag            : 1;
        add_qs_trlr             : 1;
        free_buffer             : 1;
        ascq_base               : CPU_HBM_ADDRESS_WIDTH;
        cpucb_addr              : 40;
    }
}

header_type to_stage_3_phv_t {
    fields {
        ascq_sem_inf_addr       : CPU_HBM_ADDRESS_WIDTH;
    }
}

header_type to_stage_4_phv_t {
    fields {
        src_lif                 : 16;
        asq_desc_addr           : CPU_HBM_ADDRESS_WIDTH;    
        page_addr               : CPU_HBM_ADDRESS_WIDTH;
        len                     : 16;
        vlan_tag_exists         : 1;
        tm_oq                   : 5;
    }    
}

/******************************************************************************
 * Header unions for d-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata cpu_txdma_initial_action_t cpu_txdma_initial_d;

@pragma scratch_metadata
metadata cpu_txdma_initial_action_with_pc_t cpu_txdma_initial_with_pc_d;

@pragma scratch_metadata
metadata read_asq_ci_d_t read_asq_ci_d;

@pragma scratch_metadata
metadata pkt_descr_aol_t read_asq_descr_d;

@pragma scratch_metadata
metadata cpu_to_p4plus_header_t read_cpu_hdr_d;

@pragma scratch_metadata
metadata vlan_hdr_t read_l2_vlan_hdr_d;

@pragma scratch_metadata
metadata write_pkt_d_t write_pkt_d;

/******************************************************************************
 * Header unions for PHV layout
 *****************************************************************************/
@pragma pa_header_union ingress common_global
metadata common_global_phv_t common_phv;
@pragma scratch_metadata
metadata common_global_phv_t common_global_scratch;

@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_header_t cpu_app_header;

@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;

@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;

@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;

@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;

/******************************************************************************
 * PHV following k (for app DMA etc.)
 *****************************************************************************/
@pragma dont_trim
metadata vlan_hdr_t vlan_hdr_entry;

@pragma dont_trim
metadata quiesce_pkt_trlr_t quiesce_pkt_trlr;

@pragma dont_trim
metadata ring_entry_t ascq_ring_entry; 

header_type dma_phv_pad_t {
    fields {
        dma_pad                 : 128;
    }    
}

@pragma dont_trim
metadata dma_phv_pad_t  dma_pad;

@pragma dont_trim
metadata dma_cmd_phv2pkt_t dma_cmd0;

@pragma dont_trim
metadata dma_cmd_phv2pkt_t dma_cmd1;

@pragma dont_trim
metadata dma_cmd_mem2pkt_t dma_cmd2;

@pragma dont_trim
metadata dma_cmd_phv2pkt_t dma_cmd3;

@pragma dont_trim
metadata dma_cmd_mem2pkt_t dma_cmd4;

@pragma dont_trim
metadata dma_cmd_phv2pkt_t dma_cmd5;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd_ascq;

/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/
action cpu_tx_sem_full_drop(pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
                            pi_0, ci_0, asq_base, ascq_base, ascq_sem_inf_addr,
                            asq_pi_ci_eq_drops, asq_total_pkts, ascq_sem_full_drops, ascq_free_requests) {

    GENERATE_GLOBAL_K
    // d for stage 0
    
    modify_field(cpu_txdma_initial_with_pc_d.pc, pc);
    modify_field(cpu_txdma_initial_with_pc_d.rsvd, rsvd);
    modify_field(cpu_txdma_initial_with_pc_d.cosA, cosA);
    modify_field(cpu_txdma_initial_with_pc_d.cosB, cosB);
    modify_field(cpu_txdma_initial_with_pc_d.cos_sel, cos_sel);
    modify_field(cpu_txdma_initial_with_pc_d.eval_last, eval_last);
    modify_field(cpu_txdma_initial_with_pc_d.host, host);
    modify_field(cpu_txdma_initial_with_pc_d.total, total);
    modify_field(cpu_txdma_initial_with_pc_d.pid, pid);

    modify_field(cpu_txdma_initial_with_pc_d.pi_0, pi_0);
    modify_field(cpu_txdma_initial_with_pc_d.ci_0, ci_0);
    
    modify_field(cpu_txdma_initial_with_pc_d.asq_base, asq_base);
    modify_field(cpu_txdma_initial_with_pc_d.ascq_base, ascq_base);
    modify_field(cpu_txdma_initial_with_pc_d.ascq_sem_inf_addr, ascq_sem_inf_addr);
    modify_field(cpu_txdma_initial_with_pc_d.asq_pi_ci_eq_drops, asq_pi_ci_eq_drops);
    modify_field(cpu_txdma_initial_with_pc_d.asq_total_pkts, asq_total_pkts);
    modify_field(cpu_txdma_initial_with_pc_d.ascq_sem_full_drops, ascq_sem_full_drops);
    modify_field(cpu_txdma_initial_with_pc_d.ascq_free_requests, ascq_free_requests);
}

/*
 * Stage 0 table 0 action
 */
action cpu_tx_initial_action(rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
                             pi_0, ci_0, asq_base, ascq_base, ascq_sem_inf_addr, 
                             asq_pi_ci_eq_drops, asq_total_pkts, ascq_sem_full_drops,
                             ascq_free_requests) {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);

    // d for stage 0
    
    modify_field(cpu_txdma_initial_d.rsvd, rsvd);
    modify_field(cpu_txdma_initial_d.cosA, cosA);
    modify_field(cpu_txdma_initial_d.cosB, cosB);
    modify_field(cpu_txdma_initial_d.cos_sel, cos_sel);
    modify_field(cpu_txdma_initial_d.eval_last, eval_last);
    modify_field(cpu_txdma_initial_d.host, host);
    modify_field(cpu_txdma_initial_d.total, total);
    modify_field(cpu_txdma_initial_d.pid, pid);

    modify_field(cpu_txdma_initial_d.pi_0, pi_0);
    modify_field(cpu_txdma_initial_d.ci_0, ci_0);
    
    modify_field(cpu_txdma_initial_d.asq_base, asq_base);
    modify_field(cpu_txdma_initial_d.ascq_base, ascq_base);
    modify_field(cpu_txdma_initial_d.ascq_sem_inf_addr, ascq_sem_inf_addr);
    modify_field(cpu_txdma_initial_d.asq_pi_ci_eq_drops, asq_pi_ci_eq_drops);
    modify_field(cpu_txdma_initial_d.asq_total_pkts, asq_total_pkts);
    modify_field(cpu_txdma_initial_d.ascq_sem_full_drops, ascq_sem_full_drops);
    modify_field(cpu_txdma_initial_d.ascq_free_requests, ascq_free_requests);
}

// Stage 1 table 0
action read_asq_ci(desc_addr) {
    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 1
    modify_field(read_asq_ci_d.desc_addr, desc_addr);
}

// Stage 2 table 0
action read_asq_descr(A0, O0, L0, A1, O1, L1, A2, O2, L2, next_addr, next_pkt) {
     // from ki global
    GENERATE_GLOBAL_K

    // d for stage 2
    modify_field(read_asq_descr_d.A0, A0);
    modify_field(read_asq_descr_d.O0, O0);
    modify_field(read_asq_descr_d.L0, L0);
    modify_field(read_asq_descr_d.A1, A1);
    modify_field(read_asq_descr_d.O1, O1);
    modify_field(read_asq_descr_d.L1, L1);
    modify_field(read_asq_descr_d.A2, A2);
    modify_field(read_asq_descr_d.O2, O2);
    modify_field(read_asq_descr_d.L2, L2);
    modify_field(read_asq_descr_d.next_addr, next_addr);
    modify_field(read_asq_descr_d.next_pkt, next_pkt);
}

// Stage 3 table 0
action read_cpu_hdr(flags, src_lif, hw_vlan_id, l2_offset, tm_oq) {
    // K
    modify_field(to_s3_scratch.ascq_sem_inf_addr, to_s3.ascq_sem_inf_addr);

    // d for stage 3 table 0
    modify_field(read_cpu_hdr_d.flags, flags);
    modify_field(read_cpu_hdr_d.src_lif, src_lif);
    modify_field(read_cpu_hdr_d.hw_vlan_id, hw_vlan_id);
    modify_field(read_cpu_hdr_d.l2_offset, l2_offset);
    modify_field(read_cpu_hdr_d.tm_oq, tm_oq);
}

// Stage 3 table 1
action read_l2_vlan_hdr(etherType, pcp, dei, vid) {
    // from ki global
    GENERATE_GLOBAL_K

    modify_field(read_l2_vlan_hdr_d.etherType, etherType); 
    modify_field(read_l2_vlan_hdr_d.pcp, pcp); 
    modify_field(read_l2_vlan_hdr_d.dei, dei); 
    modify_field(read_l2_vlan_hdr_d.vid, vid); 
}


// Stage 4 table 0
action write_pkt(ascq_pindex, ascq_full) {
    // from ki global
    GENERATE_GLOBAL_K

    modify_field(to_s4_scratch.src_lif, to_s4.src_lif);
    modify_field(to_s4_scratch.asq_desc_addr, to_s4.asq_desc_addr);
    modify_field(to_s4_scratch.page_addr, to_s4.page_addr);
    modify_field(to_s4_scratch.len, to_s4.len);
    modify_field(to_s4_scratch.vlan_tag_exists, to_s4.vlan_tag_exists);
    modify_field(to_s4_scratch.tm_oq, to_s4.tm_oq);

    // d-vector
    modify_field(write_pkt_d.ascq_pindex, ascq_pindex);
    modify_field(write_pkt_d.ascq_full, ascq_full);
}
