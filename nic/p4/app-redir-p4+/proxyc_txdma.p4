/*********************************************************************************
 * L7 Proxy Chain (to next service TxDMA or to P4)
 *********************************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"
#include "../cpu-p4+/cpu_rxtx_common.p4"
#include "app_redir_defines.h"
/*
 * L7 Proxy Chain stage 0
 */
#define tx_table_s0_t0_action   start


/*
 * Table names
 */
#define tx_table_s0_t0          proxyc_tx_start
#define tx_table_s1_t0          proxyc_my_txq_entry
#define tx_table_s2_t0          proxyc_meta_strip
#define tx_table_s3_t0          proxyc_desc_enqueue
#define tx_table_s3_t1          proxyc_cpu_flags_post
#define tx_table_s3_t2          proxyc_cpu_flags_skip
#define tx_table_s4_t1          proxyc_cleanup_discard
#define tx_table_s5_t0          proxyc_desc_free
#define tx_table_s5_t1          proxyc_page0_free
#define tx_table_s6_t1          proxyc_page1_free
#define tx_table_s7_t1          proxyc_page2_free

/*
 * L7 Proxy Chain stage 1
 */
#define tx_table_s1_t0_action   consume

/*
 * L7 Proxy Chain stage 2
 */
#define tx_table_s2_t0_action   meta_strip

/*
 * L7 Proxy Chain stage 3
 */
#define tx_table_s3_t0_action   desc_enqueue
#define tx_table_s3_t1_action   cpu_flags_post_read
#define tx_table_s3_t2_action   cpu_flags_skip

/*
 * L7 Proxy Chain stage 4
 */
#define tx_table_s4_t1_action   cleanup_discard

/*
 * L7 Proxy Chain stage 5
 */
#define tx_table_s5_t0_action   desc_free
#define tx_table_s5_t1_action   page0_free

/*
 * L7 Proxy Chain stage 6
 */
#define tx_table_s6_t1_action   page1_free

/*
 * L7 Proxy Chain stage 7
 */
#define tx_table_s7_t1_action   page2_free


#include "../common-p4+/common_txdma.p4"


#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.chain_txq_base, common_phv.chain_txq_base); \
    modify_field(common_global_scratch.chain_txq_ring_size_shift, common_phv.chain_txq_ring_size_shift); \
    modify_field(common_global_scratch.chain_txq_entry_size_shift, common_phv.chain_txq_entry_size_shift); \
    modify_field(common_global_scratch.chain_txq_lif, common_phv.chain_txq_lif); \
    modify_field(common_global_scratch.chain_txq_qtype, common_phv.chain_txq_qtype); \
    modify_field(common_global_scratch.chain_txq_qid, common_phv.chain_txq_qid); \
    modify_field(common_global_scratch.chain_txq_ring, common_phv.chain_txq_ring); \
    modify_field(common_global_scratch.do_cleanup_discard, common_phv.do_cleanup_discard); \
    modify_field(common_global_scratch.proxyccb_flags, common_phv.proxyccb_flags); \



/*
 * D-vectors
 */

/*
 * d for stage 0: 
 *    This is Proxy Chain CB which is also its qstate. The relevant data
 *    contain info used for enqueuing packets to the next service in the chain.
 */
header_type proxyccb_t {
    fields {
        CAPRI_QSTATE_HEADER_COMMON
        CAPRI_QSTATE_HEADER_RING(0)

        /*
         * NOTE: cb is programmed by HAL and would work best when
         * fields are aligned on whole byte boundary.
         */
         
        /*
         * Sentinel to indicate CB has been de-activated, allowing P4+ code
         * to early detect and enter cleanup.
         */
        proxyccb_deactivate             : 8;  // must be first in CB after header rings
        pad                             : 8;
        proxyccb_flags                  : 16; // DOL flags and others
        my_txq_base                     : HBM_ADDRESS_WIDTH;
        
        /*
         * Next service chain TxQ info.
         */     
        chain_txq_base                  : HBM_ADDRESS_WIDTH;
        chain_txq_ring_indices_addr     : HBM_ADDRESS_WIDTH;
        chain_txq_ring_size_shift       : 8;
        chain_txq_entry_size_shift      : 8;
        
        chain_txq_qtype                 : 8;
        chain_txq_ring                  : 8;
        chain_txq_qid                   : 32;
        chain_txq_lif                   : 16;
        
        my_txq_ring_size_shift          : 8;
        my_txq_entry_size_shift         : 8;
        
        /*
         * Sentinel to indicate all bytes in CB have been written and P4+ code
         * can start normal processing.
         */
        proxyccb_activate               : 8; // must be last in CB
    }
}

// d for stage 1 table 0
header_type txq_desc_d_t {
    fields {
        desc                            : 64;
        pad                             : 448;
    }
}

// d for stage 2 table 0 is pkt_descr_aol_t


// d for stage 3 table 0
header_type chain_txq_ring_indices_d_t {
    fields {
        CAPRI_QSTATE_HEADER_RING(curr)
        pad                             : 480;
    }
}


// d for stage 4 table 1 is also pkt_descr_aol_t

// d for stage 5 table 0
header_type sem_desc_d_t {
    fields {
        pindex_hi                       : 31;
        pindex_full                     : 1;
        pindex                          : 32;
        pad                             : 448;
    }
}

// d for stage 5 table 1
header_type sem_page_d_t {
    fields {
        pindex_hi                       : 31;
        pindex_full                     : 1;
        pindex                          : 32;
        pad                             : 448;
    }
}

// d for stage 6 table 1 is also sem_page_d_t

// d for stage 7 table 1 is also sem_page_d_t

/*
 * Global PHV definitions
 */
header_type common_global_phv_t {
    fields {
        // p4plus_common_global_t (max is GLOBAL_WIDTH or 128)
        chain_txq_base                  : HBM_ADDRESS_WIDTH;
        chain_txq_ring_size_shift       : 8;
        chain_txq_entry_size_shift      : 8;
        proxyccb_flags                  : 16;
        
        chain_txq_lif                   : 11;
        chain_txq_qtype                 : 3;
        chain_txq_qid                   : 24;
        chain_txq_ring                  : 3;
        
        do_cleanup_discard              : 1;
    }
}

header_type dma_phv_pad_128_t {
    fields {
        dma_pad                         : 128;
    }    
}


/*
 * to_stage PHV definitions
 */

header_type to_stage_1_phv_t {
    fields {
        // (max 128 bits)
        my_txq_ci_curr                  : 16;
        my_txq_ring_size_shift          : 8;
        my_txq_qid                      : 24;
        my_txq_lif                      : 11;
        my_txq_qtype                    : 3;
    }
}

header_type to_stage_2_phv_t {
    fields {
        // (max 128 bits)
        chain_txq_ring_indices_addr     : HBM_ADDRESS_WIDTH;
    }
}

header_type to_stage_3_phv_t {
    fields {
        // (max 128 bits)
        desc                            : 64;
    }
}


/*
 * Header unions for d-vector
 */
@pragma scratch_metadata
metadata proxyccb_t                     proxyccb_d;

@pragma scratch_metadata
metadata sem_desc_d_t                   sem_desc_d;

@pragma scratch_metadata
metadata sem_page_d_t                   sem_page_d;

@pragma scratch_metadata
metadata txq_desc_d_t                   my_txq_desc_d;

@pragma scratch_metadata
metadata pkt_descr_aol_t                txq_desc_aol_d;

@pragma scratch_metadata
metadata chain_txq_ring_indices_d_t     chain_txq_ring_indices_d;

@pragma scratch_metadata
metadata cpu_to_p4plus_header_t         cpu_to_p4plus_header_d;

/*
 * Stage to stage PHV definitions
 */
header_type common_t0_s2s_phv_t {
    fields {
        // (max is STAGE_2_STAGE_WIDTH or 160 bits)
        desc                    : 64;
    }
}


header_type common_t1_s2s_phv_t {
    fields {
        // (max is STAGE_2_STAGE_WIDTH or 160 bits)
        aol_A0                  : 52;
        aol_A1                  : 52;
        aol_A2                  : 52;
        aol_A0_small            : 1;
        aol_A1_small            : 1;
        aol_A2_small            : 1;
    }
}

/*
 * Header unions for PHV layout
 */
@pragma pa_header_union ingress         common_global
metadata common_global_phv_t            common_phv;
@pragma scratch_metadata
metadata common_global_phv_t            common_global_scratch;

@pragma pa_header_union ingress         app_header
metadata p4plus_to_p4_header_t          proxyc_app_header;
@pragma scratch_metadata
metadata p4plus_to_p4_header_t          proxyc_scratch_app;

@pragma pa_header_union ingress         to_stage_1
metadata to_stage_1_phv_t               to_s1;
@pragma scratch_metadata
metadata to_stage_1_phv_t               to_s1_scratch;

@pragma pa_header_union ingress         to_stage_2
metadata to_stage_2_phv_t               to_s2;
@pragma scratch_metadata
metadata to_stage_2_phv_t               to_s2_scratch;

@pragma pa_header_union ingress         to_stage_3
metadata to_stage_3_phv_t               to_s3;
@pragma scratch_metadata
metadata to_stage_3_phv_t               to_s3_scratch;

@pragma pa_header_union ingress         common_t0_s2s
metadata common_t0_s2s_phv_t            t0_s2s;
@pragma scratch_metadata
metadata common_t0_s2s_phv_t            t0_s2s_scratch;

@pragma pa_header_union ingress         common_t1_s2s
metadata common_t1_s2s_phv_t            t1_s2s;
@pragma scratch_metadata
metadata common_t1_s2s_phv_t            t1_s2s_scratch;


/*
 * PHV following k (for app DMA etc.)
 */
@pragma dont_trim
metadata ring_entry_t                   chain_txq_desc_addr; 

@pragma dont_trim
metadata doorbell_data_raw_t            chain_txq_db_data; 

/*
 * DMA descriptors for enqueuing to next service TxQ
 */
@pragma dont_trim
metadata dma_cmd_phv2mem_t              dma_chain;
@pragma dont_trim
metadata dma_cmd_phv2mem_t              dma_doorbell;

@pragma dont_trim
metadata dma_phv_pad_128_t              dma_pad_128;



/*
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 */

/*
 * Stage 0 table 0 action
 */
action start(rsvd, cosA, cosB, cos_sel, 
             eval_last, host, total, pid,
             pi_0, ci_0,
             proxyccb_deactivate,
             my_txq_base, my_txq_ring_size_shift, my_txq_entry_size_shift,
             chain_txq_ring_size_shift, chain_txq_entry_size_shift,
             chain_txq_base, chain_txq_ring_indices_addr,
             chain_txq_qid, chain_txq_lif, chain_txq_qtype, chain_txq_ring,
             proxyccb_flags, proxyccb_activate) {

    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // from p4plus_to_p4_header_t
    modify_field(proxyc_scratch_app.p4plus_app_id, proxyc_app_header.p4plus_app_id);
    modify_field(proxyc_scratch_app.flags, proxyc_app_header.flags);
    modify_field(proxyc_scratch_app.ip_id_delta, proxyc_app_header.ip_id_delta);
    modify_field(proxyc_scratch_app.tcp_seq_delta, proxyc_app_header.tcp_seq_delta);
    modify_field(proxyc_scratch_app.vlan_tag, proxyc_app_header.vlan_tag);
    
    // d for stage 0
    
    //modify_field(proxyccb_d.pc, pc);
    modify_field(proxyccb_d.rsvd, rsvd);
    modify_field(proxyccb_d.cosA, cosA);
    modify_field(proxyccb_d.cosB, cosB);
    modify_field(proxyccb_d.cos_sel, cos_sel);
    modify_field(proxyccb_d.eval_last, eval_last);
    modify_field(proxyccb_d.host, host);
    modify_field(proxyccb_d.total, total);
    modify_field(proxyccb_d.pid, pid);
    modify_field(proxyccb_d.pi_0, pi_0);
    modify_field(proxyccb_d.ci_0, ci_0);
    
    modify_field(proxyccb_d.proxyccb_deactivate, proxyccb_deactivate);
    modify_field(proxyccb_d.my_txq_base, my_txq_base);
    modify_field(proxyccb_d.my_txq_ring_size_shift, my_txq_ring_size_shift);
    modify_field(proxyccb_d.my_txq_entry_size_shift, my_txq_entry_size_shift);
    modify_field(proxyccb_d.chain_txq_base, chain_txq_base);
    modify_field(proxyccb_d.chain_txq_ring_indices_addr, chain_txq_ring_indices_addr);
    modify_field(proxyccb_d.chain_txq_ring_size_shift, chain_txq_ring_size_shift);
    modify_field(proxyccb_d.chain_txq_entry_size_shift, chain_txq_entry_size_shift);
    modify_field(proxyccb_d.chain_txq_lif, chain_txq_lif);
    modify_field(proxyccb_d.chain_txq_qtype, chain_txq_qtype);
    modify_field(proxyccb_d.chain_txq_qid, chain_txq_qid);
    modify_field(proxyccb_d.chain_txq_ring, chain_txq_ring);
    modify_field(proxyccb_d.proxyccb_flags, proxyccb_flags);
    modify_field(proxyccb_d.proxyccb_activate, proxyccb_activate);
}


/*
 * Stage 1 table 0 action
 */
action consume(desc) {

    // from to_stage
    modify_field(to_s1_scratch.my_txq_ci_curr, to_s1.my_txq_ci_curr);
    modify_field(to_s1_scratch.my_txq_ring_size_shift, to_s1.my_txq_ring_size_shift);
    modify_field(to_s1_scratch.my_txq_lif, to_s1.my_txq_lif);
    modify_field(to_s1_scratch.my_txq_qtype, to_s1.my_txq_qtype);
    modify_field(to_s1_scratch.my_txq_qid, to_s1.my_txq_qid);
    
    // from ki global
    GENERATE_GLOBAL_K
    
    // d for stage and table
    modify_field(my_txq_desc_d.desc, desc);
}


/*
 * Stage 2 table 0 action
 */
action meta_strip(A0, O0, L0,
                  A1, O1, L1,
                  A2, O2, L2) {

    // from to_stage
    modify_field(to_s2_scratch.chain_txq_ring_indices_addr, to_s2.chain_txq_ring_indices_addr);
    
    // from ki global
    GENERATE_GLOBAL_K
    
    // from stage to stage
    
    // d for stage and table
    modify_field(txq_desc_aol_d.A0, A0);
    modify_field(txq_desc_aol_d.O0, O0);
    modify_field(txq_desc_aol_d.L0, L0);
    modify_field(txq_desc_aol_d.A1, A1);
    modify_field(txq_desc_aol_d.O1, O1);
    modify_field(txq_desc_aol_d.L1, L1);
    modify_field(txq_desc_aol_d.A2, A2);
    modify_field(txq_desc_aol_d.O2, O2);
    modify_field(txq_desc_aol_d.L2, L2);
}


/*
 * Stage 3 table 0 action
 */
action desc_enqueue(pi_curr, ci_curr) {

    // from to_stage
    modify_field(to_s3_scratch.desc, to_s3.desc);
    
    // from ki global
    GENERATE_GLOBAL_K
    
    // from stage to stage
    
    // d for stage and table
    modify_field(chain_txq_ring_indices_d.pi_curr, pi_curr);
    modify_field(chain_txq_ring_indices_d.ci_curr, ci_curr);
}


/*
 * Stage 3 table 1 action
 */
action cpu_flags_post_read(flags, src_lif, hw_vlan_id, l2_offset) {

    // from to_stage
    modify_field(to_s3_scratch.desc, to_s3.desc);
    
    // from ki global
    GENERATE_GLOBAL_K
    
    // from stage to stage
    modify_field(t1_s2s_scratch.aol_A0, t1_s2s.aol_A0);
    modify_field(t1_s2s_scratch.aol_A1, t1_s2s.aol_A1);
    modify_field(t1_s2s_scratch.aol_A2, t1_s2s.aol_A2);
    modify_field(t1_s2s_scratch.aol_A0_small, t1_s2s.aol_A0_small);
    modify_field(t1_s2s_scratch.aol_A1_small, t1_s2s.aol_A1_small);
    modify_field(t1_s2s_scratch.aol_A2_small, t1_s2s.aol_A2_small);
    
    // d for stage and table
    modify_field(cpu_to_p4plus_header_d.flags, flags);
    modify_field(cpu_to_p4plus_header_d.src_lif, src_lif);
    modify_field(cpu_to_p4plus_header_d.hw_vlan_id, hw_vlan_id);
    modify_field(cpu_to_p4plus_header_d.l2_offset, l2_offset);
}


/*
 * Stage 3 table 2 action
 */
action cpu_flags_skip() {

    // from to_stage
    modify_field(to_s3_scratch.desc, to_s3.desc);
    
    // from ki global
    GENERATE_GLOBAL_K
    
    // from stage to stage
    
    // d for stage and table
}


/*
 * Stage 4 table 1 action
 */
action cleanup_discard(A0, O0, L0,
                       A1, O1, L1,
                       A2, O2, L2) {

    // k + i for stage 4

    // from to_stage 4

    // from ki global
    GENERATE_GLOBAL_K
    
    // from stage to stage
    modify_field(t1_s2s_scratch.aol_A0, t1_s2s.aol_A0);
    modify_field(t1_s2s_scratch.aol_A1, t1_s2s.aol_A1);
    modify_field(t1_s2s_scratch.aol_A2, t1_s2s.aol_A2);
    modify_field(t1_s2s_scratch.aol_A0_small, t1_s2s.aol_A0_small);
    modify_field(t1_s2s_scratch.aol_A1_small, t1_s2s.aol_A1_small);
    modify_field(t1_s2s_scratch.aol_A2_small, t1_s2s.aol_A2_small);
    
    // d for stage 4 table 1
    modify_field(txq_desc_aol_d.A0, A0);
    modify_field(txq_desc_aol_d.O0, O0);
    modify_field(txq_desc_aol_d.L0, L0);
    modify_field(txq_desc_aol_d.A1, A1);
    modify_field(txq_desc_aol_d.O1, O1);
    modify_field(txq_desc_aol_d.L1, L1);
    modify_field(txq_desc_aol_d.A2, A2);
    modify_field(txq_desc_aol_d.O2, O2);
    modify_field(txq_desc_aol_d.L2, L2);
}


/*
 * Stage 5 table 0 action
 */
action desc_free(pindex, pindex_full) {
    // k + i for stage 5 table 0
    
    // from to_stage 5

    // from ki global
    //GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(t0_s2s_scratch.desc, t0_s2s.desc);

    // d for stage 5 table 0
    modify_field(sem_desc_d.pindex, pindex);
    modify_field(sem_desc_d.pindex_full, pindex_full);
}


/*
 * Stage 5 table 1 action
 */
action page0_free(pindex, pindex_full) {
    // k + i for stage 5 table 1
    
    // from to_stage 5

    // from ki global
    //GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(t1_s2s_scratch.aol_A0, t1_s2s.aol_A0);
    modify_field(t1_s2s_scratch.aol_A1, t1_s2s.aol_A1);
    modify_field(t1_s2s_scratch.aol_A2, t1_s2s.aol_A2);
    modify_field(t1_s2s_scratch.aol_A0_small, t1_s2s.aol_A0_small);
    modify_field(t1_s2s_scratch.aol_A1_small, t1_s2s.aol_A1_small);
    modify_field(t1_s2s_scratch.aol_A2_small, t1_s2s.aol_A2_small);

    // d for stage 7 table 1
    modify_field(sem_page_d.pindex, pindex);
    modify_field(sem_page_d.pindex_full, pindex_full);
}


/*
 * Stage 6 table 1 action
 */
action page1_free(pindex, pindex_full) {
    // k + i for stage 6 table 1
    
    // from to_stage 6

    // from ki global
    //GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(t1_s2s_scratch.aol_A0, t1_s2s.aol_A0);
    modify_field(t1_s2s_scratch.aol_A1, t1_s2s.aol_A1);
    modify_field(t1_s2s_scratch.aol_A2, t1_s2s.aol_A2);
    modify_field(t1_s2s_scratch.aol_A0_small, t1_s2s.aol_A0_small);
    modify_field(t1_s2s_scratch.aol_A1_small, t1_s2s.aol_A1_small);
    modify_field(t1_s2s_scratch.aol_A2_small, t1_s2s.aol_A2_small);

    // d for stage 6 table 1
    modify_field(sem_page_d.pindex, pindex);
    modify_field(sem_page_d.pindex_full, pindex_full);
}


/*
 * Stage 7 table 1 action
 */
action page2_free(pindex, pindex_full) {
    // k + i for stage 7 table 1
    
    // from to_stage 7

    // from ki global
    //GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(t1_s2s_scratch.aol_A0, t1_s2s.aol_A0);
    modify_field(t1_s2s_scratch.aol_A1, t1_s2s.aol_A1);
    modify_field(t1_s2s_scratch.aol_A2, t1_s2s.aol_A2);
    modify_field(t1_s2s_scratch.aol_A0_small, t1_s2s.aol_A0_small);
    modify_field(t1_s2s_scratch.aol_A1_small, t1_s2s.aol_A1_small);
    modify_field(t1_s2s_scratch.aol_A2_small, t1_s2s.aol_A2_small);

    // d for stage 7 table 1
    modify_field(sem_page_d.pindex, pindex);
    modify_field(sem_page_d.pindex_full, pindex_full);
}


