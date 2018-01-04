/*********************************************************************************
 * L7 Raw Redirect
 *********************************************************************************/

#include "../common-p4+/common_rxdma_dummy.p4"
#include "app_redir_defines.h"

/*
 * L7 Raw Redirect stage 0
 */
#define rx_table_s0_t0_action   rawr_rx_start
#define common_p4plus_stage0_app_header_table_action_dummy rawr_rx_start


/*
 * Table names
 */
#define rx_table_s1_t0          rawr_desc_sem
#define rx_table_s1_t1          rawr_ppage_sem
#define rx_table_s1_t2          rawr_mpage_sem

#define rx_table_s2_t0          rawr_desc
#define rx_table_s2_t1          rawr_ppage
#define rx_table_s2_t2          rawr_mpage

#define rx_table_s5_t0          rawr_chain_pindex_pre_alloc

#define rx_table_s6_t0          rawr_cleanup_discard
#define rx_table_s6_t1          rawr_chain_pindex

#define rx_table_s7_t0          rawr_desc_free
#define rx_table_s7_t1          rawr_ppage_free
#define rx_table_s7_t2          rawr_mpage_free
#define rx_table_s7_t3          rawr_stats              // actually stage agnostic

/*
 * L7 Raw Redirect stage 1
 */
#define rx_table_s1_t0_action   desc_pindex_post_update 
#define rx_table_s1_t1_action   ppage_pindex_post_update 
#define rx_table_s1_t2_action   mpage_pindex_post_update 

/*
 * L7 Raw Redirect stage 2
 */
#define rx_table_s2_t0_action   desc_post_alloc
#define rx_table_s2_t1_action   ppage_post_alloc
#define rx_table_s2_t2_action   mpage_post_alloc

/*
 * L7 Raw Redirect stage 3
 */
#define rx_table_s3_t0_action   chain_qidxr_stage_advance_a

/*
 * L7 Raw Redirect stage 4
 */
#define rx_table_s4_t0_action   chain_qidxr_stage_advance_b

/*
 * L7 Raw Redirect stage 5
 */
#define rx_table_s5_t0_action   chain_pindex_pre_alloc

/*
 * L7 Raw Redirect stage 6
 */
#define rx_table_s6_t0_action   cleanup_discard
#define rx_table_s6_t1_action   chain_xfer
#define rx_table_s6_t1_action1  txq_post_read
#define rx_table_s6_t1_action2  qidxr_post_read

/*
 * L7 Raw Redirect stage 7
 */
#define rx_table_s7_t0_action   desc_free
#define rx_table_s7_t1_action   ppage_free
#define rx_table_s7_t2_action   mpage_free
#define rx_table_s7_t3_action   err_stat_inc    // actually stage agnostic


#include "../common-p4+/common_rxdma.p4"
#include "../cpu-p4+/cpu_rx_common.p4"


/*
 * D-vectors
 */

/*
 * d for stage 0: 
 *    This is Raw Redirect CB which is also its qstate. The relevant data
 *    contain info used for enqueuing packets to the next service in the chain.
 *    In other words, the data includes a pointer to the next service's
 *    qstate entry.
 */
header_type rawrcb_t {
    fields {
        CAPRI_QSTATE_HEADER_COMMON

        /*
         * NOTE: cb is programmed by HAL and would be best when
         * fields are aligned on whole byte boundary.
         */
         
        /*
         * Sentinel to indicate CB has been de-activated, allowing P4+ code
         * to early detect and enter cleanup.
         */
        rawrcb_deactivate               : 8;  // must be first in CB after header common
        pad                             : 8;
        rawrcb_flags                    : 16; // DOL flags and others
         
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
        
        chain_rxq_base                  : 64;
        chain_rxq_ring_indices_addr     : 64;
        
        /*
         * Next service chain TxQ info, if any, for redirection type (b):
         * packet descriptors should be enqueued to the given TxQ
         * and the corresponding doorbell rung.
         */     
        chain_txq_ring_indices_addr     : 64;
        
        chain_txq_base                  : 64;
        chain_txq_qid                   : 32;
        chain_txq_lif                   : 16;
        chain_txq_qtype                 : 8;
        chain_txq_ring_size_shift       : 8;
        chain_txq_entry_size_shift      : 8;
        chain_txq_ring_index_select     : 8;
        
        chain_rxq_ring_size_shift       : 8;
        chain_rxq_entry_size_shift      : 8;
        chain_rxq_ring_index_select     : 8;
        
        /*
         * Sentinel to indicate all bytes in CB have been written and P4+ code
         * can start normal processing.
         */
        rawrcb_activate                 : 8; // must be last in CB
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

// d for stage 1 table 1
header_type sem_ppage_d_t {
    fields {
        pindex_hi                       : 31;
        pindex_full                     : 1;
        pindex                          : 32;
        pad                             : 448;
    }
}

// d for stage 1 table 2
header_type sem_mpage_d_t {
    fields {
        pindex_hi                       : 31;
        pindex_full                     : 1;
        pindex                          : 32;
        pad                             : 448;
    }
}

// d for stage 2 table 0
header_type alloc_desc_d_t {
    fields {
        desc                            : 64;
        pad                             : 448;
    }
}

// d for stage 2 table 1
header_type alloc_ppage_d_t {
    fields {
        page                            : 64;
        pad                             : 448;
    }
}

// d for stage 2 table 2
header_type alloc_mpage_d_t {
    fields {
        page                            : 64;
        pad                             : 448;
    }
}

// d for stage 6 table 1
header_type chain_txq_ring_indices_d_t {
    fields {
        CAPRI_QSTATE_HEADER_RING(curr)
        pad                             : 480;
    }
}

// d for stage x table 3
header_type rawrcb_stats_t {
    fields {
        stat_pkts_redir                 : 64;
        stat_pkts_discard               : 64;

        // 32-bit saturating statistic counters
        stat_cb_not_ready               : 32;
        stat_qstate_cfg_err             : 32;
        stat_pkt_len_err                : 32;
        stat_rxq_full                   : 32;
        stat_txq_full                   : 32;
        stat_desc_sem_alloc_full        : 32;
        stat_ppage_sem_alloc_full       : 32;
        stat_mpage_sem_alloc_full       : 32;
        stat_sem_free_full              : 32;
        unused0                         : 32;
        unused1                         : 32;
        unused2                         : 32;
    }
}

/*
 * Global PHV definitions
 */
header_type common_global_phv_t {
    fields {
        // p4plus_common_global_t (max is GLOBAL_WIDTH or 128)
        chain_to_rxq                    : 1;
        redir_span_instance             : 1;
        desc_sem_pindex_full            : 1;
        ppage_sem_pindex_full           : 1;
        mpage_sem_pindex_full           : 1;
        chain_ring_index_select         : 3;
        rawrcb_flags                    : 16;
        packet_len                      : 16;
        chain_ring_base                 : 34;
        chain_ring_size_shift           : 5;
        qstate_addr                     : 34;
        chain_entry_size_shift          : 5;
    }
}

#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.chain_ring_base, common_phv.chain_ring_base); \
    modify_field(common_global_scratch.packet_len, common_phv.packet_len); \
    modify_field(common_global_scratch.chain_to_rxq, common_phv.chain_to_rxq); \
    modify_field(common_global_scratch.desc_sem_pindex_full, common_phv.desc_sem_pindex_full); \
    modify_field(common_global_scratch.ppage_sem_pindex_full, common_phv.ppage_sem_pindex_full); \
    modify_field(common_global_scratch.mpage_sem_pindex_full, common_phv.mpage_sem_pindex_full); \
    modify_field(common_global_scratch.chain_ring_size_shift, common_phv.chain_ring_size_shift); \
    modify_field(common_global_scratch.chain_entry_size_shift, common_phv.chain_entry_size_shift); \
    modify_field(common_global_scratch.chain_ring_index_select, common_phv.chain_ring_index_select); \
    modify_field(common_global_scratch.redir_span_instance, common_phv.redir_span_instance); \
    modify_field(common_global_scratch.rawrcb_flags, common_phv.rawrcb_flags); \
    modify_field(common_global_scratch.qstate_addr, common_phv.qstate_addr);


/*
 * to_stage PHV definitions
 */

header_type to_stage_5_phv_t {
    fields {
        // (max 128 bits)
        chain_ring_indices_addr         : 34;
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
metadata rawrcb_t                       rawrcb_d;

@pragma scratch_metadata
metadata sem_desc_d_t                   sem_desc_d;

@pragma scratch_metadata
metadata sem_ppage_d_t                  sem_ppage_d;

@pragma scratch_metadata
metadata sem_mpage_d_t                  sem_mpage_d;

@pragma scratch_metadata
metadata chain_txq_ring_indices_d_t     chain_txq_ring_indices_d;

@pragma scratch_metadata
metadata alloc_desc_d_t                 alloc_desc_d;

@pragma scratch_metadata
metadata alloc_ppage_d_t                alloc_ppage_d;

@pragma scratch_metadata
metadata alloc_mpage_d_t                alloc_mpage_d;

@pragma scratch_metadata
metadata arq_pi_d_t                     qidxr_chain_d;

@pragma scratch_metadata
metadata rawrcb_stats_t                 rawrcb_stats_d;

/*
 * Stage to stage PHV definitions
 */
header_type common_t1_s2s_phv_t {
    fields {
        // (max is STAGE_2_STAGE_WIDTH or 160 bits)
        chain_lif               : 11;
        chain_qtype             : 3;
        chain_qid               : 24;
    }
}

#define GENERATE_T1_S2S_K \
    modify_field(t1_s2s_scratch.chain_lif, t1_s2s.chain_lif); \
    modify_field(t1_s2s_scratch.chain_qtype, t1_s2s.chain_qtype); \
    modify_field(t1_s2s_scratch.chain_qid, t1_s2s.chain_qid);
    

header_type common_t3_s2s_phv_t {
    fields {
        // (max is STAGE_2_STAGE_WIDTH or 160 bits)
        inc_stat_begin                : 1;
        inc_stat_pkts_redir           : 1;
        inc_stat_pkts_discard         : 1;
        inc_stat_cb_not_ready         : 1;
        inc_stat_qstate_cfg_err       : 1;
        inc_stat_pkt_len_err          : 1;
        inc_stat_rxq_full             : 1;
        inc_stat_txq_full             : 1;
        inc_stat_desc_sem_alloc_full  : 1;
        inc_stat_ppage_sem_alloc_full : 1;
        inc_stat_mpage_sem_alloc_full : 1;
        inc_stat_sem_free_full        : 1;
        inc_stat_current_all          : 1;
        inc_stat_end                  : 1;
    }
}

#define GENERATE_T3_S2S_K \
    modify_field(t3_s2s_scratch.inc_stat_begin, t3_s2s.inc_stat_begin); \
    modify_field(t3_s2s_scratch.inc_stat_pkts_redir, t3_s2s.inc_stat_pkts_redir); \
    modify_field(t3_s2s_scratch.inc_stat_pkts_discard, t3_s2s.inc_stat_pkts_discard); \
    modify_field(t3_s2s_scratch.inc_stat_cb_not_ready, t3_s2s.inc_stat_cb_not_ready); \
    modify_field(t3_s2s_scratch.inc_stat_qstate_cfg_err, t3_s2s.inc_stat_qstate_cfg_err); \
    modify_field(t3_s2s_scratch.inc_stat_pkt_len_err, t3_s2s.inc_stat_pkt_len_err); \
    modify_field(t3_s2s_scratch.inc_stat_rxq_full, t3_s2s.inc_stat_rxq_full); \
    modify_field(t3_s2s_scratch.inc_stat_txq_full, t3_s2s.inc_stat_txq_full); \
    modify_field(t3_s2s_scratch.inc_stat_desc_sem_alloc_full, t3_s2s.inc_stat_desc_sem_alloc_full); \
    modify_field(t3_s2s_scratch.inc_stat_ppage_sem_alloc_full, t3_s2s.inc_stat_ppage_sem_alloc_full); \
    modify_field(t3_s2s_scratch.inc_stat_mpage_sem_alloc_full, t3_s2s.inc_stat_mpage_sem_alloc_full); \
    modify_field(t3_s2s_scratch.inc_stat_sem_free_full, t3_s2s.inc_stat_sem_free_full); \
    modify_field(t3_s2s_scratch.inc_stat_current_all, t3_s2s.inc_stat_current_all); \
    modify_field(t3_s2s_scratch.inc_stat_end, t3_s2s.inc_stat_end);
    

/*
 * Header unions for PHV layout
 */
@pragma pa_header_union ingress         common_global
metadata common_global_phv_t            common_phv;
@pragma scratch_metadata
metadata common_global_phv_t            common_global_scratch;

@pragma pa_header_union ingress         app_header
metadata p4_to_p4plus_cpu_header_t      rawr_app_header;
@pragma scratch_metadata
metadata p4_to_p4plus_cpu_header_t      rawr_scratch_app;

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

@pragma dont_trim
@pragma pa_header_union ingress         common_t1_s2s
metadata common_t1_s2s_phv_t            t1_s2s;
@pragma scratch_metadata
metadata common_t1_s2s_phv_t            t1_s2s_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         common_t3_s2s
metadata common_t3_s2s_phv_t            t3_s2s;
@pragma scratch_metadata
metadata common_t3_s2s_phv_t            t3_s2s_scratch;

/*
 * PHV following k (for app DMA etc.)
 */
@pragma dont_trim
metadata pen_app_redir_header_t         pen_app_redir_hdr;

@pragma dont_trim
metadata pen_app_redir_version_header_t pen_app_redir_version_hdr;

@pragma dont_trim
metadata pen_raw_redir_header_v1_t      pen_raw_redir_hdr_v1;

@pragma dont_trim
metadata ring_entry_t                   ring_entry; 

@pragma dont_trim
metadata doorbell_data_raw_t            chain_txq_db_data; 

@pragma dont_trim
@pragma pa_align 512
metadata pkt_descr_t                    aol; 

@pragma dont_trim
@pragma pa_align 512
metadata dma_cmd_pkt2mem_t              dma_cpu_pkt;
@pragma dont_trim
metadata dma_cmd_phv2mem_t              dma_meta;
@pragma dont_trim
metadata dma_cmd_pkt2mem_t              dma_pkt_content;
@pragma dont_trim
metadata dma_cmd_phv2mem_t              dma_desc;
@pragma dont_trim
metadata dma_cmd_phv2mem_t              dma_chain;
@pragma dont_trim
metadata dma_cmd_phv2mem_t              dma_doorbell;


/*
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 */

/*
 * Stage 0 table 0 action
 */
action rawr_rx_start(rsvd, cosA, cosB, cos_sel, 
                     eval_last, host, total, pid,
                     rawrcb_deactivate, chain_rxq_base,
                     chain_rxq_ring_indices_addr, chain_rxq_ring_size_shift,
                     chain_rxq_entry_size_shift, chain_rxq_ring_index_select,
                     chain_txq_base, chain_txq_ring_indices_addr,
                     chain_txq_ring_size_shift, chain_txq_entry_size_shift,
                     chain_txq_lif, chain_txq_qtype, chain_txq_qid,
                     chain_txq_ring_index_select,
                     rawrcb_flags, rawrcb_activate) {
                     
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    
    // temporarily remove access to tm_instance_type until issue of qid..qtype
    // range is solved in RDMA
    // modify_field(p4_intr_global_scratch.tm_instance_type, p4_intr_global.tm_instance_type);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from cpu p4_to_p4plus_cpu_header_t
    modify_field(rawr_scratch_app.p4plus_app_id, rawr_app_header.p4plus_app_id);
    modify_field(rawr_scratch_app.table0_valid, rawr_app_header.table0_valid);
    modify_field(rawr_scratch_app.table1_valid, rawr_app_header.table1_valid);
    modify_field(rawr_scratch_app.table2_valid, rawr_app_header.table2_valid);
    modify_field(rawr_scratch_app.table3_valid, rawr_app_header.table3_valid);
    modify_field(rawr_scratch_app.ip_proto, rawr_app_header.ip_proto);
    modify_field(rawr_scratch_app.l4_sport, rawr_app_header.l4_sport);
    modify_field(rawr_scratch_app.l4_dport, rawr_app_header.l4_dport);
    
    // packet_len is length of packet plus sizeof(p4_to_p4plus_cpu_pkt_t)
    // as set by P4 apps.p4
    modify_field(rawr_scratch_app.packet_len, rawr_app_header.packet_len);

    // from stage to stage
    
    // d for stage 0
    
    //modify_field(rawrcb_d.pc, pc);
    modify_field(rawrcb_d.rsvd, rsvd);
    modify_field(rawrcb_d.cosA, cosA);
    modify_field(rawrcb_d.cosB, cosB);
    modify_field(rawrcb_d.cos_sel, cos_sel);
    modify_field(rawrcb_d.eval_last, eval_last);
    modify_field(rawrcb_d.host, host);
    modify_field(rawrcb_d.total, total);
    modify_field(rawrcb_d.pid, pid);
    
    modify_field(rawrcb_d.rawrcb_deactivate, rawrcb_deactivate);
    modify_field(rawrcb_d.chain_rxq_base, chain_rxq_base);
    modify_field(rawrcb_d.chain_rxq_ring_indices_addr, chain_rxq_ring_indices_addr);
    modify_field(rawrcb_d.chain_rxq_ring_size_shift, chain_rxq_ring_size_shift);
    modify_field(rawrcb_d.chain_rxq_entry_size_shift, chain_rxq_entry_size_shift);
    modify_field(rawrcb_d.chain_rxq_ring_index_select, chain_rxq_ring_index_select);
    modify_field(rawrcb_d.chain_txq_base, chain_txq_base);
    modify_field(rawrcb_d.chain_txq_ring_indices_addr, chain_txq_ring_indices_addr);
    modify_field(rawrcb_d.chain_txq_ring_size_shift, chain_txq_ring_size_shift);
    modify_field(rawrcb_d.chain_txq_entry_size_shift, chain_txq_entry_size_shift);
    modify_field(rawrcb_d.chain_txq_lif, chain_txq_lif);
    modify_field(rawrcb_d.chain_txq_qtype, chain_txq_qtype);
    modify_field(rawrcb_d.chain_txq_qid, chain_txq_qid);
    modify_field(rawrcb_d.chain_txq_ring_index_select, chain_txq_ring_index_select);
    modify_field(rawrcb_d.rawrcb_flags, rawrcb_flags);
    modify_field(rawrcb_d.rawrcb_activate, rawrcb_activate);
}

/*
 * Stage 1 table 0 action
 */
action desc_pindex_post_update(pindex, pindex_full) {
    
    // d for stage
    modify_field(sem_desc_d.pindex, pindex);
    modify_field(sem_desc_d.pindex_full, pindex_full);
}

/*
 * Stage 1 table 1 action
 */
action ppage_pindex_post_update(pindex, pindex_full) {
    
    // d for stage
    modify_field(sem_ppage_d.pindex, pindex);
    modify_field(sem_ppage_d.pindex_full, pindex_full);
}

/*
 * Stage 1 table 2 action
 */
action mpage_pindex_post_update(pindex, pindex_full) {
    
    // d for stage
    modify_field(sem_mpage_d.pindex, pindex);
    modify_field(sem_mpage_d.pindex_full, pindex_full);
    
}

/*
 * Stage 2 table 0 action
 */
action desc_post_alloc(desc, pad) {
    // k + i for stage 2 table 0

    // from to_stage 2
    
    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 2 table 0
    modify_field(alloc_desc_d.desc, desc);
    modify_field(alloc_desc_d.pad, pad);
}

/*
 * Stage 2 table 1 action
 */
action ppage_post_alloc(page, pad) {
    // k + i for stage 2 table 1

    // from to_stage 2
    
    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 2 table 1
    modify_field(alloc_ppage_d.page, page);
    modify_field(alloc_ppage_d.pad, pad);
}

/*
 * Stage 2 table 2 action
 */
action mpage_post_alloc(page, pad) {
    // k + i for stage 2 table 2

    // from to_stage 2
    
    // from ki global
    //GENERATE_GLOBAL_K

    // d for stage 2 table 2
    modify_field(alloc_mpage_d.page, page);
    modify_field(alloc_mpage_d.pad, pad);
}

/*
 * Stage 3 table 0 action
 */
action chain_qidxr_stage_advance_a() {
}

/*
 * Stage 4 table 0 action
 */
action chain_qidxr_stage_advance_b() {
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
action txq_post_read(pi_curr, ci_curr) {
    // k + i for stage 6 table 1

    // from to_stage 6

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 6 table 1
    modify_field(chain_txq_ring_indices_d.pi_curr, pi_curr);
    modify_field(chain_txq_ring_indices_d.ci_curr, ci_curr);
}


/*
 * Stage 6 table 1 action1
 */
action chain_qidxr_pindex_post_read(ARQ_PI_PARAMS) {
    // k + i for stage 6 table 2

    // from to_stage 6

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 6 table 1
    GENERATE_ARQ_PI_D(qidxr_chain_d)
}


/*
 * Stage 6 table 1 action2
 */
action chain_xfer() {

    // k + i for stage 6

    // from to_stage 6
    modify_field(to_s6_scratch.desc,  to_s6.desc);
    modify_field(to_s6_scratch.ppage, to_s6.ppage);
    modify_field(to_s6_scratch.mpage, to_s6.mpage);

    // from ki global
    GENERATE_GLOBAL_K
    
    // from stage to stage
    GENERATE_T1_S2S_K
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
    // k + i for stage 7 table 1
    
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

/*
 * Stage agnostic table 3 action.
 * Caution: must not be launched from stage 7.
 */
action err_stat_inc(stat_pkts_redir,
                    stat_pkts_discard,
                    stat_cb_not_ready,
                    stat_qstate_cfg_err,
                    stat_pkt_len_err,
                    stat_rxq_full,
                    stat_txq_full,
                    stat_desc_sem_alloc_full,
                    stat_ppage_sem_alloc_full,
                    stat_mpage_sem_alloc_full,
                    stat_sem_free_full,
                    unused0, unused1, unused2) {
    // k + i for stage x table 3
    
    // from to_stage x

    // from ki global
    //GENERATE_GLOBAL_K

    // from stage to stage
    GENERATE_T3_S2S_K

    // d for stage x table 3
    modify_field(rawrcb_stats_d.stat_pkts_redir, stat_pkts_redir);
    modify_field(rawrcb_stats_d.stat_pkts_discard, stat_pkts_discard);
    modify_field(rawrcb_stats_d.stat_cb_not_ready, stat_cb_not_ready);
    modify_field(rawrcb_stats_d.stat_qstate_cfg_err, stat_qstate_cfg_err);
    modify_field(rawrcb_stats_d.stat_pkt_len_err, stat_pkt_len_err);
    modify_field(rawrcb_stats_d.stat_rxq_full, stat_rxq_full);
    modify_field(rawrcb_stats_d.stat_txq_full, stat_txq_full);
    modify_field(rawrcb_stats_d.stat_desc_sem_alloc_full, stat_desc_sem_alloc_full);
    modify_field(rawrcb_stats_d.stat_ppage_sem_alloc_full, stat_ppage_sem_alloc_full);
    modify_field(rawrcb_stats_d.stat_mpage_sem_alloc_full, stat_mpage_sem_alloc_full);
    modify_field(rawrcb_stats_d.stat_sem_free_full, stat_sem_free_full);
    modify_field(rawrcb_stats_d.unused0, unused0);
    modify_field(rawrcb_stats_d.unused1, unused1);
    modify_field(rawrcb_stats_d.unused2, unused2);
}

