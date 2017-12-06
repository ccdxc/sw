/*********************************************************************************
 * L7 Proxy Redirect
 *********************************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"
#include "app_redir_defines.h"

/*
 * L7 Proxy Redirect stage 0
 */
#define tx_table_s0_t0_action   start


/*
 * Table names
 */
#define tx_table_s0_t0          proxyr_tx_start
#define tx_table_s1_t0          proxyr_my_txq_entry
#define tx_table_s1_t1          proxyr_flow_key
#define tx_table_s2_t0          proxyr_desc
#define tx_table_s3_t0          proxyr_mpage_sem_post
#define tx_table_s3_t1          proxyr_mpage_sem_skip
#define tx_table_s4_t0          proxyr_mpage_post
#define tx_table_s4_t1          proxyr_mpage_skip
#define tx_table_s5_t0          proxyr_chain_pindex
#define tx_table_s6_t0          proxyr_cleanup_discard
#define tx_table_s6_t1          proxyr_chain_xfer
#define tx_table_s7_t0          proxyr_desc_free
#define tx_table_s7_t1          proxyr_ppage_free
#define tx_table_s7_t2          proxyr_mpage_free

/*
 * L7 proxy redirect stage 1
 */
#define tx_table_s1_t0_action   consume
#define tx_table_s1_t1_action   flow_key_post_read

/*
 * L7 Proxy Chain stage 2
 */
#define tx_table_s2_t0_action   desc_post_read

/*
 * L7 Proxy Chain stage 3
 */
#define tx_table_s3_t0_action   mpage_pindex_post_update
#define tx_table_s3_t1_action   mpage_pindex_skip

/*
 * L7 Proxy Chain stage 4
 */
#define tx_table_s4_t0_action   mpage_post_alloc
#define tx_table_s4_t1_action   mpage_skip_alloc

/*
 * L7 Proxy Chain stage 5
 */
#define tx_table_s5_t0_action   chain_pindex_pre_alloc

/*
 * L7 Proxy Chain stage 6
 */
#define tx_table_s6_t0_action   cleanup_discard
#define tx_table_s6_t1_action   chain_xfer

/*
 * L7 Proxy Chain stage 7
 */
#define tx_table_s7_t0_action   desc_free
#define tx_table_s7_t1_action   ppage_free
#define tx_table_s7_t2_action   mpage_free


#include "../common-p4+/common_txdma.p4"
#include "../cpu-p4+/cpu_rx_common.p4"


#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.chain_ring_base, common_phv.chain_ring_base); \
    modify_field(common_global_scratch.do_cleanup_discard, common_phv.do_cleanup_discard); \
    modify_field(common_global_scratch.mpage_sem_pindex_full, common_phv.mpage_sem_pindex_full); \
    modify_field(common_global_scratch.redir_span_instance, common_phv.redir_span_instance); \
    modify_field(common_global_scratch.chain_ring_size_shift, common_phv.chain_ring_size_shift); \
    modify_field(common_global_scratch.chain_entry_size_shift, common_phv.chain_entry_size_shift); \
    modify_field(common_global_scratch.chain_ring_index_select, common_phv.chain_ring_index_select); \
    modify_field(common_global_scratch.proxyrcb_flags, common_phv.proxyrcb_flags); \



/*
 * D-vectors
 */

/*
 * d for stage 0: 
 *    This is Proxy Redirect CB which is also its qstate. The relevant data
 *    contain info used for enqueuing packets to the next service in the chain.
 *    In other words, the data includes a pointer to the next service's
 *    qstate entry.
 */
header_type proxyrcb_t {
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
        proxyrcb_deactivate             : 8;  // must be first in CB after header rings
        pad                             : 8;
        proxyrcb_flags                  : 16; // DOL flags and others
        my_txq_base                     : HBM_ADDRESS_WIDTH;

        /*
         * For a given flow, one of 2 types of redirection can apply:
         *   a) Redirect to ARM CPU RxQ, or
         *   b) Redirect to a P4+ TxQ
         *
         * Service chain RxQs that are ARM CPU bound (e.g. ARQs) cannot use
         * semaphores because the ARM does not have HW support for semaphore
         * manipulation. In such cases, special HBM queue index regions are
         * provided for direct access under table lock to prevent race condition.
         *
         * Note: table lock is only effective for a given stage so all P4+
         * programs must coordinate so that they lock a given table in
         * the same stage. For the ARM ARQ, that is stage 6.
         */     
        chain_rxq_base                  : HBM_ADDRESS_WIDTH;
        chain_rxq_ring_indices_addr     : HBM_ADDRESS_WIDTH;
        chain_rxq_ring_size_shift       : 8;
        chain_rxq_entry_size_shift      : 8;
        chain_rxq_ring_index_select     : 8;
        
        my_txq_ring_size_shift          : 8;
        my_txq_entry_size_shift         : 8;
    }
}

/*
 * Next 64 bytes in CB
 */
header_type proxyrcb_flow_key_t {
    fields {
    
        /*
         * NOTE: cb is programmed by HAL and would work best when
         * fields are aligned on whole byte boundary.
         *
         * The following static flow key fields (from vrf to ip_proto)
         * must be sized and ordered exactly as defined in the structure
         * pen_proxy_redir_header_v1_t in 
         * asm/app-redir-p4+/common/include/app_redir_headers.h
         */
        vrf                             : 16;
        ip_sa                           : 128;
        ip_da                           : 128;
        sport                           : 16;
        dport                           : 16;
        af                              : 8;
        ip_proto                        : 8;
        
        /*
         * Sentinel to indicate all bytes in CB have been written and P4+ code
         * can start normal processing.
         */
        proxyrcb_activate               : 8; // must be last in CB
    }
}

// d for stage 1 table 0
header_type txq_desc_d_t {
    fields {
        desc                            : 64;
        pad                             : 448;
    }
}


// d for stage 1 table 0
header_type sem_desc_d_t {
    fields {
        pindex_hi                       : 31;
        pindex_full                     : 1;
        pindex                          : 32;
        pad                             : 448;
    }
}

// d for stage 2 table 0 is pkt_descr_aol_t


// d for stage 3 table 0
header_type sem_mpage_d_t {
    fields {
        pindex_hi                       : 31;
        pindex_full                     : 1;
        pindex                          : 32;
        pad                             : 448;
    }
}

// d for stage 4 table 0
header_type alloc_mpage_d_t {
    fields {
        page                            : 64;
        pad                             : 448;
    }
}


// d for stage 6 table 1 is arq_rx_pi_d_t

// d for stage 7 table 1
header_type sem_ppage_d_t {
    fields {
        pindex_hi                       : 31;
        pindex_full                     : 1;
        pindex                          : 32;
        pad                             : 448;
    }
}

/*
 * Global PHV definitions
 */
header_type common_global_phv_t {
    fields {
        // p4plus_common_global_t (max is GLOBAL_WIDTH or 128)
        chain_ring_base                 : HBM_ADDRESS_WIDTH;
        chain_ring_size_shift           : 5;
        chain_entry_size_shift          : 5;
        chain_ring_index_select         : 3;
        do_cleanup_discard              : 1;
        mpage_sem_pindex_full           : 1;
        redir_span_instance             : 1;
        proxyrcb_flags                  : 16;
    }
}

header_type header_phv_pad_224_t {
    fields {
        hdr_pad                         : 224;
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
        proxyrcb_deactivate             : 8;
        my_txq_ring_size_shift          : 8;
        my_txq_ci_curr                  : 16;
        my_txq_qid                      : 24;
        my_txq_lif                      : 11;
        my_txq_qtype                    : 3;
    }
}


header_type to_stage_5_phv_t {
    fields {
        // (max 128 bits)
        chain_ring_indices_addr         : HBM_ADDRESS_WIDTH;
    }
}

header_type to_stage_6_phv_t {
    fields {
        // (max 128 bits)
        desc                            : 34;
        ppage                           : 34;
        mpage                           : 34;
    }
}

header_type to_stage_7_phv_t {
    fields {
        // (max 128 bits)
        desc                            : 34;
        ppage                           : 34;
        mpage                           : 34;
    }
}


/*
 * Header unions for d-vector
 */
@pragma scratch_metadata
metadata proxyrcb_t                     proxyrcb_d;

@pragma scratch_metadata
metadata proxyrcb_flow_key_t            proxyrcb_flow_key_d;

@pragma scratch_metadata
metadata sem_desc_d_t                   sem_desc_d;

@pragma scratch_metadata
metadata txq_desc_d_t                   my_txq_desc_d;

@pragma scratch_metadata
metadata sem_mpage_d_t                  sem_mpage_d;

@pragma scratch_metadata
metadata sem_ppage_d_t                  sem_ppage_d;

@pragma scratch_metadata
metadata alloc_mpage_d_t                alloc_mpage_d;

@pragma scratch_metadata
metadata arq_rx_pi_d_t                  qidxr_chain_d;

@pragma scratch_metadata
metadata pkt_descr_aol_t                desc_aol_d;


/*
 * Stage to stage PHV definitions
 */


/*
 * Header unions for PHV layout
 */
@pragma pa_header_union ingress         common_global
metadata common_global_phv_t            common_phv;
@pragma scratch_metadata
metadata common_global_phv_t            common_global_scratch;

@pragma pa_header_union ingress         app_header
metadata p4plus_to_p4_header_t          proxyr_app_header;
@pragma scratch_metadata
metadata p4plus_to_p4_header_t          proxyr_scratch_app;

@pragma pa_header_union ingress         to_stage_1
metadata to_stage_1_phv_t               to_s1;
@pragma scratch_metadata
metadata to_stage_1_phv_t               to_s1_scratch;

@pragma pa_header_union ingress         to_stage_5
metadata to_stage_5_phv_t               to_s5;
@pragma scratch_metadata
metadata to_stage_5_phv_t               to_s5_scratch;

@pragma pa_header_union ingress         to_stage_6
metadata to_stage_6_phv_t               to_s6;
@pragma scratch_metadata
metadata to_stage_6_phv_t               to_s6_scratch;

@pragma pa_header_union ingress         to_stage_7
metadata to_stage_7_phv_t               to_s7;
@pragma scratch_metadata
metadata to_stage_7_phv_t               to_s7_scratch;


/*
 * PHV following k (for app DMA etc.)
 */
 
@pragma dont_trim
metadata p4_to_p4plus_cpu_pkt_t         p4plus_cpu_pkt;

/*
 * The above p4plus_cpu_pkt + pen_app_redir_hdr + pen_app_redir_version_hdr +
 * pen_proxyr_hdr_v1 would cause pen_proxyr_hdr_v1 to be split across 2 flits
 * with some intervening bits. Hence, insert a pad to push L7 headers
 * to a completely new flit.
 */
@pragma dont_trim
metadata header_phv_pad_224_t           header_phv_pad_224;

@pragma dont_trim
metadata pen_app_redir_header_t         pen_app_redir_hdr;

@pragma dont_trim
metadata pen_app_redir_version_header_t pen_app_redir_version_hdr;

@pragma dont_trim
metadata pen_proxy_redir_header_v1_t    pen_proxyr_hdr_v1;

@pragma dont_trim
metadata ring_entry_t                   ring_entry; 

@pragma dont_trim
metadata pkt_descr_t                    aol;

/*
 * DMA descriptors for redirecting
 */
@pragma dont_trim
metadata dma_cmd_phv2mem_t              dma_meta;
@pragma dont_trim
metadata dma_cmd_phv2mem_t              dma_proxyr_hdr;
@pragma dont_trim
metadata dma_cmd_phv2mem_t              dma_desc;
@pragma dont_trim
metadata dma_cmd_phv2mem_t              dma_chain;

//@pragma dont_trim
//metadata dma_phv_pad_128_t              dma_pad_128;



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
             proxyrcb_deactivate,
             my_txq_base, my_txq_ring_size_shift, my_txq_entry_size_shift,
             chain_rxq_base, chain_rxq_ring_indices_addr,
             chain_rxq_ring_size_shift, chain_rxq_entry_size_shift,
             chain_rxq_ring_index_select, 
             proxyrcb_flags) {

    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // d for stage 0
    
    //modify_field(proxyrcb_d.pc, pc);
    modify_field(proxyrcb_d.rsvd, rsvd);
    modify_field(proxyrcb_d.cosA, cosA);
    modify_field(proxyrcb_d.cosB, cosB);
    modify_field(proxyrcb_d.cos_sel, cos_sel);
    modify_field(proxyrcb_d.eval_last, eval_last);
    modify_field(proxyrcb_d.host, host);
    modify_field(proxyrcb_d.total, total);
    modify_field(proxyrcb_d.pid, pid);
    modify_field(proxyrcb_d.pi_0, pi_0);
    modify_field(proxyrcb_d.ci_0, ci_0);
    
    modify_field(proxyrcb_d.proxyrcb_deactivate, proxyrcb_deactivate);
    modify_field(proxyrcb_d.my_txq_base, my_txq_base);
    modify_field(proxyrcb_d.my_txq_ring_size_shift, my_txq_ring_size_shift);
    modify_field(proxyrcb_d.my_txq_entry_size_shift, my_txq_entry_size_shift);
    modify_field(proxyrcb_d.chain_rxq_base, chain_rxq_base);
    modify_field(proxyrcb_d.chain_rxq_ring_indices_addr, chain_rxq_ring_indices_addr);
    modify_field(proxyrcb_d.chain_rxq_ring_size_shift, chain_rxq_ring_size_shift);
    modify_field(proxyrcb_d.chain_rxq_entry_size_shift, chain_rxq_entry_size_shift);
    modify_field(proxyrcb_d.chain_rxq_ring_index_select, chain_rxq_ring_index_select);
    modify_field(proxyrcb_d.proxyrcb_flags, proxyrcb_flags);
}


/*
 * Stage 1 table 0 action
 */
action consume(desc) {

    // from to_stage
    modify_field(to_s1_scratch.proxyrcb_deactivate, to_s1.proxyrcb_deactivate);
    modify_field(to_s1_scratch.my_txq_ring_size_shift, to_s1.my_txq_ring_size_shift);
    modify_field(to_s1_scratch.my_txq_ci_curr, to_s1.my_txq_ci_curr);
    modify_field(to_s1_scratch.my_txq_lif, to_s1.my_txq_lif);
    modify_field(to_s1_scratch.my_txq_qtype, to_s1.my_txq_qtype);
    modify_field(to_s1_scratch.my_txq_qid, to_s1.my_txq_qid);
    
    // from ki global
    GENERATE_GLOBAL_K
    
    // d for stage and table
    modify_field(my_txq_desc_d.desc, desc);
}


/*
 * Stage 1 table 1 action
 */
action flow_key_post_read(ip_sa, ip_da, sport, dport, 
                          vrf, af, ip_proto,
                          proxyrcb_activate) {

    // k + i for stage 1 table 1

    // from to_stage 1
    modify_field(to_s1_scratch.proxyrcb_deactivate, to_s1.proxyrcb_deactivate);
    modify_field(to_s1_scratch.my_txq_ring_size_shift, to_s1.my_txq_ring_size_shift);
    modify_field(to_s1_scratch.my_txq_ci_curr, to_s1.my_txq_ci_curr);
    modify_field(to_s1_scratch.my_txq_lif, to_s1.my_txq_lif);
    modify_field(to_s1_scratch.my_txq_qtype, to_s1.my_txq_qtype);
    modify_field(to_s1_scratch.my_txq_qid, to_s1.my_txq_qid);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 1 table 1
    modify_field(proxyrcb_flow_key_d.ip_sa, ip_sa);
    modify_field(proxyrcb_flow_key_d.ip_da, ip_da);
    modify_field(proxyrcb_flow_key_d.sport, sport);
    modify_field(proxyrcb_flow_key_d.dport, dport);
    modify_field(proxyrcb_flow_key_d.vrf, vrf);
    modify_field(proxyrcb_flow_key_d.af, af);
    modify_field(proxyrcb_flow_key_d.ip_proto, ip_proto);
    modify_field(proxyrcb_flow_key_d.proxyrcb_activate, proxyrcb_activate);
}


/*
 * Stage 2 table 0 action
 */
action desc_post_read(A0, O0, L0,
                      A1, O1, L1,
                      A2, O2, L2,
                      next_addr, next_pkt) {

    // from to_stage
    //modify_field(to_s6_scratch.ppage, to_s6.ppage);
    
    // from ki global
    GENERATE_GLOBAL_K
    
    // from stage to stage
    
    // d for stage and table
    modify_field(desc_aol_d.A0, A0);
    modify_field(desc_aol_d.O0, O0);
    modify_field(desc_aol_d.L0, L0);
    modify_field(desc_aol_d.A1, A1);
    modify_field(desc_aol_d.O1, O1);
    modify_field(desc_aol_d.L1, L1);
    modify_field(desc_aol_d.A2, A2);
    modify_field(desc_aol_d.O2, O2);
    modify_field(desc_aol_d.L2, L2);
    modify_field(desc_aol_d.next_addr, next_addr);
    modify_field(desc_aol_d.next_pkt, next_pkt);
}


/*
 * Stage 3 table 0 action
 */
action mpage_pindex_post_update(pindex, pindex_full) {
    modify_field(sem_mpage_d.pindex, pindex);
    modify_field(sem_mpage_d.pindex_full, pindex_full);
}


/*
 * Stage 3 table 1 action
 */
action mpage_pindex_skip() {
    // k + i for stage 3 table 1

    // from to_stage 3
    
    // from ki global
    //GENERATE_GLOBAL_K

    // d for stage 3 table 1
}


/*
 * Stage 4 table 0 action
 */
action mpage_post_alloc(page, pad) {
    // k + i for stage 4 table 0

    // from to_stage 4
    
    // from ki global
    //GENERATE_GLOBAL_K

    // d for stage 4 table 0
    modify_field(alloc_mpage_d.page, page);
    modify_field(alloc_mpage_d.pad, pad);
}


/*
 * Stage 4 table 1 action
 */
action mpage_skip_alloc() {

    // k + i for stage 4 table 1

    // from to_stage 4
    
    // from ki global
    //GENERATE_GLOBAL_K

    // d for stage 4 table 1
}


/*
 * Stage 5 table 0 action
 */
action chain_pindex_pre_alloc() {

    // k + i for stage 5 table 0

    // from to_stage 5
    modify_field(to_s5_scratch.chain_ring_indices_addr, to_s5.chain_ring_indices_addr);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 5 table 0
}


/*
 * Stage 6 table 0 action
 */
action cleanup_discard() {

    // k + i for stage 6

    // from to_stage 6
    modify_field(to_s6_scratch.desc,  to_s6.desc);
    modify_field(to_s6_scratch.ppage, to_s6.ppage);
    modify_field(to_s6_scratch.mpage, to_s6.mpage);

    // from ki global
    GENERATE_GLOBAL_K
    
    // d for stage 6 table 0
}


/*
 * Stage 6 table 1 action
 */
action chain_xfer(pi_0, pi_1, pi_2) {

    // k + i for stage 6 table 1

    // from to_stage 6

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(to_s6_scratch.desc,  to_s6.desc);
    modify_field(to_s6_scratch.ppage, to_s6.ppage);
    modify_field(to_s6_scratch.mpage, to_s6.mpage);

    // d for stage 6 table 2
    modify_field(qidxr_chain_d.pi_0, pi_0);
    modify_field(qidxr_chain_d.pi_1, pi_1);
    modify_field(qidxr_chain_d.pi_2, pi_2);
}


/*
 * Stage 7 table 0 action
 */
action desc_free(pindex, pindex_full) {

    // k + i for stage 7 table 0
    
    // from to_stage 7
    modify_field(to_s7_scratch.desc, to_s7.desc);

    // from ki global
    //GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 7 table 0
    modify_field(sem_desc_d.pindex, pindex);
    modify_field(sem_desc_d.pindex_full, pindex_full);
}


/*
 * Stage 7 table 1 action
 */
action ppage_free(pindex, pindex_full) {

    // k + i for stage 7 table 2
    
    // from to_stage 7
    modify_field(to_s7_scratch.ppage, to_s7.ppage);

    // from ki global
    //GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 7 table 1
    modify_field(sem_ppage_d.pindex, pindex);
    modify_field(sem_ppage_d.pindex_full, pindex_full);
}


/*
 * Stage 7 table 2 action
 */
action mpage_free(pindex, pindex_full) {

    // k + i for stage 7 table 2
    
    // from to_stage 7
    modify_field(to_s7_scratch.mpage, to_s7.mpage);

    // from ki global
    //GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 7 table 2
    modify_field(sem_mpage_d.pindex, pindex);
    modify_field(sem_mpage_d.pindex_full, pindex_full);
}

