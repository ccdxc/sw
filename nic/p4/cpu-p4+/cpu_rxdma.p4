/*********************************************************************************
 * cpu_rxdma_actions.p4
 * This file implements p4+ program in RXDMA for handling CPU bound packets
 *********************************************************************************/
#include "../common-p4+/common_rxdma_dummy.p4"
#include "cpu_rxtx_common.p4"

/*******************************************************
 * Table and actions
 ******************************************************/

#define rx_table_s0_t0_action cpu_rxdma_initial_action
#define common_p4plus_stage0_app_header_table_action_dummy cpu_rxdma_initial_action

#define rx_table_s1_t1 cpu_rx_read_cpu_desc
#define rx_table_s1_t1_action read_cpu_desc 
#define rx_table_s1_t2 cpu_rx_read_cpu_page
#define rx_table_s1_t2_action read_cpu_page 

#define rx_table_s2_t1 cpu_rx_desc_alloc
#define rx_table_s2_t1_action desc_alloc
#define rx_table_s2_t2 cpu_rx_page_alloc
#define rx_table_s2_t2_action page_alloc

// Stage 3 is for hash defined in common rxdma

#define rx_table_s4_t0 cpu_rx_read_arqrx
#define rx_table_s4_t0_action read_arqrx 

#define rx_table_s5_t0 cpu_rx_write_arqrx
#define rx_table_s5_t0_action write_arqrx

#include "../common-p4+/common_rxdma.p4"
#include "cpu_rx_common.p4"

#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.qstate_addr, common_phv.qstate_addr); \
    modify_field(common_global_scratch.flags, common_phv.flags); \
    modify_field(common_global_scratch.debug_dol, common_phv.debug_dol); \

/******************************************************************************
 * D-vectors
 *****************************************************************************/

// d for stage 0
header_type cpu_rxdma_initial_action_t {
    fields {
        // 8 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        flags                       : 8;
        debug_dol                   : 8;
        pad                         : 424;
    }
}

// d for stage 1 table 1
header_type read_cpudr_d_t {
    fields {
        desc_pindex         : 32;
        desc_pindex_full    : 1; 
    }
}

// d for stage 1 table 2
header_type read_cpupr_d_t {
    fields {
        page_pindex         : 32;
        page_pindex_full    : 1;
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

/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/
header_type p4_to_p4plus_cpu_header_ext_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        ip_proto            : 8;
        l4_sport            : 16;
        l4_dport            : 16;
        packet_type         : 2;
        packet_len          : 14;
        ip_sa               : 128;
        ip_da1              : 64;
        ip_da2              : 64;
    }
}

header_type common_global_phv_t {
    fields {
        // global k (max 128)
        qstate_addr             : 32;
        flags                   : 8;
        debug_dol               : 8;
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
header_type common_t0_s2s_phv_t {
    fields {
        page                    : 32;
        descr                   : 32;
	    arqrx_base              : 32;
        arqrx_pindex            : 16;
        payload_len             : 16;
        hash                    : 32;
    }
}

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
metadata desc_alloc_d_t desc_alloc_d;

@pragma scratch_metadata
metadata page_alloc_d_t page_alloc_d;

@pragma scratch_metadata
metadata arq_rx_pi_d_t write_arqrx_d;

/******************************************************************************
 * Header unions for PHV layout
 *****************************************************************************/
@pragma pa_header_union ingress common_global
metadata common_global_phv_t common_phv;
 @pragma scratch_metadata
metadata common_global_phv_t common_global_scratch;

@pragma pa_header_union ingress app_header
metadata p4_to_p4plus_cpu_header_ext_t cpu_app_header;
@pragma scratch_metadata
metadata p4_to_p4plus_cpu_header_ext_t cpu_scratch_app;

@pragma scratch_metadata
metadata s2_t1_s2s_phv_t s2_t1_s2s_scratch;
@pragma pa_header_union ingress common_t1_s2s
metadata s2_t1_s2s_phv_t s2_t1_s2s;

@pragma scratch_metadata
metadata s2_t2_s2s_phv_t s2_t2_s2s_scratch;
@pragma pa_header_union ingress common_t2_s2s
metadata s2_t2_s2s_phv_t s2_t2_s2s;

@pragma pa_header_union ingress common_t0_s2s
metadata common_t0_s2s_phv_t t0_s2s;
@pragma scratch_metadata
metadata common_t0_s2s_phv_t t0_s2s_scratch;

/******************************************************************************
 * PHV following k (for app DMA etc.)
 *****************************************************************************/

@pragma dont_trim
metadata ring_entry_t ring_entry; 

@pragma dont_trim
metadata quiesce_pkt_trlr_t quiesce_pkt_trlr;

@pragma dont_trim
metadata pkt_descr_aol_t aol; 

@pragma dont_trim
metadata dma_cmd_pkt2mem_t dma_cmd0;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd1;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd2;

@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd3;

/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/
/*
 * Stage 0 table 0 action
 */
action cpu_rxdma_initial_action(rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, flags, debug_dol, pad) {
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
    modify_field(cpu_scratch_app.ip_proto, cpu_app_header.ip_proto);
    modify_field(cpu_scratch_app.l4_sport, cpu_app_header.l4_sport);
    modify_field(cpu_scratch_app.l4_dport, cpu_app_header.l4_dport);
    modify_field(cpu_scratch_app.packet_type, cpu_app_header.packet_type);
    modify_field(cpu_scratch_app.packet_len, cpu_app_header.packet_len);
    modify_field(cpu_scratch_app.ip_sa, cpu_app_header.ip_sa);
    modify_field(cpu_scratch_app.ip_da1, cpu_app_header.ip_da1);
    modify_field(cpu_scratch_app.ip_da2, cpu_app_header.ip_da2);

    // d for stage 0
    modify_field(cpu_rxdma_initial_d.rsvd, rsvd);
    modify_field(cpu_rxdma_initial_d.cosA, cosA);
    modify_field(cpu_rxdma_initial_d.cosB, cosB);
    modify_field(cpu_rxdma_initial_d.cos_sel, cos_sel);
    modify_field(cpu_rxdma_initial_d.eval_last, eval_last);
    modify_field(cpu_rxdma_initial_d.host, host);
    modify_field(cpu_rxdma_initial_d.total, total);
    modify_field(cpu_rxdma_initial_d.pid, pid);
    modify_field(cpu_rxdma_initial_d.flags, flags);
    modify_field(cpu_rxdma_initial_d.debug_dol, debug_dol);
    modify_field(cpu_rxdma_initial_d.pad, pad);
}

// Stage 1 table 1 action
action read_cpu_desc(desc_pindex, desc_pindex_full) {
    // d for stage 1 table 1 
    modify_field(read_cpudr_d.desc_pindex, desc_pindex);
    modify_field(read_cpudr_d.desc_pindex_full, desc_pindex_full);
}

// Stage 1 table 2 action
action read_cpu_page(page_pindex, page_pindex_full) {
    // d for stage 1 table 2 
    modify_field(read_cpupr_d.page_pindex, page_pindex);
    modify_field(read_cpupr_d.page_pindex_full, page_pindex_full);
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

// Stage 4 table 0 action
action read_arqrx() {
}


/*
 * Stage 5 table 0 action
 */
action write_arqrx(pi_0, pi_1, pi_2) {
    // k + i for stage 3

    // from t0_s2s
    modify_field(t0_s2s_scratch.page, t0_s2s.page);
    modify_field(t0_s2s_scratch.descr, t0_s2s.descr);
    modify_field(t0_s2s_scratch.arqrx_pindex, t0_s2s.arqrx_pindex);
    modify_field(t0_s2s_scratch.arqrx_base, t0_s2s.arqrx_base);
    modify_field(t0_s2s_scratch.payload_len, t0_s2s.payload_len);
    modify_field(t0_s2s_scratch.hash, t0_s2s.hash);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 2 to stage 3

    // d for stage 3 table 0
    modify_field(write_arqrx_d.pi_0, pi_0);
    modify_field(write_arqrx_d.pi_1, pi_1);
    modify_field(write_arqrx_d.pi_2, pi_2);
}

