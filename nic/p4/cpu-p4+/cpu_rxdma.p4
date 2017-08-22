/*********************************************************************************
 * cpu_rxdma_actions.p4
 * This file implements p4+ program in RXDMA for handling CPU bound packets
 *********************************************************************************/

#define common_p4plus_stage0_app_header_table_action_dummy1 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy2 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy3 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy4 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy5 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy6 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy7 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy8 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy9 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy10 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy11 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy12 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy13 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy14 cpu_rx_dummy_action
#define common_p4plus_stage0_app_header_table_action_dummy15 cpu_rx_dummy_action


#define rx_table_s0_t0_action cpu_rxdma_initial_action
#define common_p4plus_stage0_app_header_table_action_dummy cpu_rxdma_initial_action
#define rx_table_s0_t1_action cpu_rx_dummy_action
#define rx_table_s0_t2_action cpu_rx_dummy_action
#define rx_table_s0_t3_action cpu_rx_dummy_action

#define rx_table_s1_t1 cpu_rx_read_cpu_desc
#define rx_table_s1_t2 cpu_rx_read_cpu_page
#define rx_table_s1_t3 cpu_rx_read_arqrx

#define rx_table_s1_t0_action cpu_rx_dummy_action
#define rx_table_s1_t1_action read_cpu_desc 
#define rx_table_s1_t2_action read_cpu_page 
#define rx_table_s1_t3_action read_arqrx 

#define rx_table_s2_t1 cpu_rx_desc_alloc
#define rx_table_s2_t2 cpu_rx_page_alloc

#define rx_table_s2_t0_action cpu_rx_dummy_action
#define rx_table_s2_t1_action desc_alloc
#define rx_table_s2_t2_action page_alloc
#define rx_table_s2_t3_action cpu_rx_dummy_action

#define rx_table_s3_t0 cpu_rx_write_arqrx

#define rx_table_s3_t0_action write_arqrx
#define rx_table_s3_t1_action cpu_rx_dummy_action
#define rx_table_s3_t2_action cpu_rx_dummy_action
#define rx_table_s3_t3_action cpu_rx_dummy_action

#define rx_table_s4_t0_action cpu_rx_dummy_action
#define rx_table_s4_t1_action cpu_rx_dummy_action
#define rx_table_s4_t2_action cpu_rx_dummy_action
#define rx_table_s4_t3_action cpu_rx_dummy_action

#define rx_table_s5_t0_action cpu_rx_dummy_action
#define rx_table_s5_t1_action cpu_rx_dummy_action
#define rx_table_s5_t2_action cpu_rx_dummy_action
#define rx_table_s5_t3_action cpu_rx_dummy_action

#define rx_table_s6_t0_action cpu_rx_dummy_action 
#define rx_table_s6_t1_action cpu_rx_dummy_action
#define rx_table_s6_t2_action cpu_rx_dummy_action
#define rx_table_s6_t3_action cpu_rx_dummy_action

#define rx_table_s7_t0_action cpu_rx_dummy_action
#define rx_table_s7_t1_action cpu_rx_dummy_action
#define rx_table_s7_t2_action cpu_rx_dummy_action
#define rx_table_s7_t3_action cpu_rx_dummy_action

#include "../common-p4+/common_rxdma.p4"
#include "cpu_defines.h"

#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.qstate_addr, common_phv.qstate_addr); \

/******************************************************************************
 * D-vectors
 *****************************************************************************/

// d for stage 0
header_type cpu_rxdma_initial_action_t {
    fields {
        // 8 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON

        pad                         : 440;
    }
}

// d for stage 1 table 1
header_type read_cpudr_d_t {
    fields {
        desc_pindex    : RING_INDEX_WIDTH;
    }
}

// d for stage 1 table 2
header_type read_cpupr_d_t {
    fields {
        page_pindex    : RING_INDEX_WIDTH;
    }
}

// d for stage 1 table 3
header_type read_arqrx_d_t {
    fields {
        arqrx_pindex    : RING_INDEX_WIDTH;
    }
}

// d for stage 2 table 1
header_type desc_alloc_d_t {
    fields {
        desc                    : 64;
        pad                     : 448;
    }
}

// d for stage 2 table 2
header_type page_alloc_d_t {
    fields {
        page                    : 64;
        pad                     : 448;
    }
}

// d for stage 3 table 0
header_type write_arqrx_d_t {
    fields {
        nde_addr                : 64;
        nde_offset              : 16;
        nde_len                 : 16;
        curr_ts                 : 32;
    }
}

/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/
header_type to_stage_3_phv_t {
    // write-serq
    fields {
        page                    : 32;
        descr                   : 32;
	    arqrx_base              : 32;
        arqrx_pindex            : 16;
        payload_len             : 16;
    }
}

header_type common_global_phv_t {
    fields {
        // global k (max 128)
        qstate_addr             : 32;
    }
}

header_type dma_phv_pad_t {
    fields {
        dma_pad                 : 448;    
    }    
}
/******************************************************************************
 * Stage to stage PHV definitions
 *****************************************************************************/

header_type s2_t1_s2s_phv_t {
    fields {
        desc_pindex             : 16;
    }
}
header_type s2_t2_s2s_phv_t {
    fields {
        page_pindex             : 16;
    }
}

/******************************************************************************
 * Header unions for d-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata cpu_rxdma_initial_action_t cpu_rxdma_initial_d;

@pragma scratch_metadata
metadata read_cpudr_d_t read_cpudr_d;

@pragma scratch_metadata
metadata read_cpupr_d_t read_cpupr_d;

@pragma scratch_metadata
metadata read_arqrx_d_t read_arqrx_d;

@pragma scratch_metadata
metadata desc_alloc_d_t desc_alloc_d;

@pragma scratch_metadata
metadata page_alloc_d_t page_alloc_d;

@pragma scratch_metadata
metadata write_arqrx_d_t write_arqrx_d;

/******************************************************************************
 * Header unions for PHV layout
 *****************************************************************************/
@pragma pa_header_union ingress common_global
metadata common_global_phv_t common_phv;
 @pragma scratch_metadata
metadata common_global_phv_t common_global_scratch;

@pragma pa_header_union ingress app_header
metadata p4_to_p4plus_cpu_header_t cpu_app_header;
@pragma scratch_metadata
metadata p4_to_p4plus_cpu_header_t cpu_scratch_app;

@pragma scratch_metadata
metadata s2_t1_s2s_phv_t s2_t1_s2s_scratch;
@pragma pa_header_union ingress common_t1_s2s
metadata s2_t1_s2s_phv_t s2_t1_s2s;

@pragma scratch_metadata
metadata s2_t2_s2s_phv_t s2_t2_s2s_scratch;
@pragma pa_header_union ingress common_t2_s2s
metadata s2_t2_s2s_phv_t s2_t2_s2s;

@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;
@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;

/******************************************************************************
 * PHV following k (for app DMA etc.)
 *****************************************************************************/

@pragma dont_trim
metadata ring_entry_t ring_entry; 

@pragma dont_trim
metadata dma_phv_pad_t  dma_pad;

@pragma dont_trim
metadata pkt_descr_t aol; 

@pragma dont_trim
metadata dma_cmd_pkt2mem_t dma_cmd0;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd1;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd2;

/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/

// Dummy
action cpu_rx_dummy_action() 
{
}

/*
 * Stage 0 table 0 action
 */
action cpu_rxdma_initial_action(pc, rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pad) {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(cpu_scratch_app.p4plus_app_id, cpu_app_header.p4plus_app_id);
    modify_field(cpu_scratch_app.table0_valid, cpu_app_header.table0_valid);
    modify_field(cpu_scratch_app.table1_valid, cpu_app_header.table1_valid);
    modify_field(cpu_scratch_app.table2_valid, cpu_app_header.table2_valid);
    modify_field(cpu_scratch_app.table3_valid, cpu_app_header.table3_valid);
    modify_field(cpu_scratch_app.packet_len, cpu_app_header.packet_len);
    modify_field(cpu_scratch_app.flow_hash, cpu_app_header.flow_hash);

    // d for stage 0
    
    modify_field(cpu_rxdma_initial_d.pc, pc);
    modify_field(cpu_rxdma_initial_d.rsvd, rsvd);
    modify_field(cpu_rxdma_initial_d.cosA, cosA);
    modify_field(cpu_rxdma_initial_d.cosB, cosB);
    modify_field(cpu_rxdma_initial_d.cos_sel, cos_sel);
    modify_field(cpu_rxdma_initial_d.eval_last, eval_last);
    modify_field(cpu_rxdma_initial_d.host, host);
    modify_field(cpu_rxdma_initial_d.total, total);
    modify_field(cpu_rxdma_initial_d.pid, pid);
    modify_field(cpu_rxdma_initial_d.pad, pad);
}

// Stage 1 table 1 action
action read_cpu_desc(desc_pindex) {
    // d for stage 1 table 1 
    modify_field(read_cpudr_d.desc_pindex, desc_pindex);
}

// Stage 1 table 2 action
action read_cpu_page(page_pindex) {
    // d for stage 1 table 2 
    modify_field(read_cpupr_d.page_pindex, page_pindex);
}

// Stage 1 table 3 action
action read_arqrx(arqrx_pindex) {
    // d for srage 2 table 3
    modify_field(read_arqrx_d.arqrx_pindex, arqrx_pindex);
}

/*
 * Stage 2 table 1 action
 */
action desc_alloc(desc, pad) {
    // k + i for stage 2 table 1

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 1 to stage 2
    modify_field(s2_t1_s2s_scratch.desc_pindex, s2_t1_s2s.desc_pindex);

    // d for stage 2 table 1
    modify_field(desc_alloc_d.desc, desc);
    modify_field(desc_alloc_d.pad, pad);
}

/*
 * Stage 2 table 2 action
 */
action page_alloc(page, pad) {
    // k + i for stage 3 table 2

    // from stage 1 to stage 2
    modify_field(s2_t2_s2s_scratch.page_pindex, s2_t2_s2s.page_pindex);

    // d for stage 3 table 2
    modify_field(page_alloc_d.page, page);
    modify_field(page_alloc_d.pad, pad);
}

/*
 * Stage 3 table 0 action
 */
action write_arqrx(nde_addr, nde_offset, nde_len, curr_ts) {
    // k + i for stage 3

    // from to_stage 3
    modify_field(to_s3_scratch.page, to_s3.page);
    modify_field(to_s3_scratch.descr, to_s3.descr);
    modify_field(to_s3_scratch.arqrx_pindex, to_s3.arqrx_pindex);
    modify_field(to_s3_scratch.arqrx_base, to_s3.arqrx_base);
    modify_field(to_s3_scratch.payload_len, to_s3.payload_len);

    // from ki global
    //GENERATE_GLOBAL_K

    // from stage 2 to stage 3

    // d for stage 3 table 0
    modify_field(write_arqrx_d.nde_addr, nde_addr);
    modify_field(write_arqrx_d.nde_offset, nde_offset);
    modify_field(write_arqrx_d.nde_len, nde_len);
    modify_field(write_arqrx_d.curr_ts, curr_ts);
}

