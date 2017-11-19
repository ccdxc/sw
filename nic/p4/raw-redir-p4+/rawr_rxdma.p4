/*********************************************************************************
 * L7 Raw Redirect
 *********************************************************************************/

#include "../common-p4+/common_rxdma_dummy.p4"
#include "raw_redir_defines.h"

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
#define rx_table_s6_t1          rawr_chain_txq_pindex
#define rx_table_s6_t2          rawr_chain_qidxr
#define rx_table_s6_t3          rawr_chain_xfer

#define rx_table_s7_t0          rawr_desc_free
#define rx_table_s7_t1          rawr_ppage_free
#define rx_table_s7_t2          rawr_mpage_free

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
#define rx_table_s6_t1_action   chain_txq_pindex_post_read
#define rx_table_s6_t2_action   chain_qidxr_pindex_post_read
#define rx_table_s6_t3_action   chain_xfer

/*
 * L7 Raw Redirect stage 7
 */
#define rx_table_s7_t0_action   desc_free
#define rx_table_s7_t1_action   ppage_free
#define rx_table_s7_t2_action   mpage_free

#include "../common-p4+/common_rxdma.p4"
#include "../cpu-p4+/cpu_rx_common.p4"


#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.chain_ring_base, common_phv.chain_ring_base); \
    modify_field(common_global_scratch.packet_len, common_phv.packet_len); \
    modify_field(common_global_scratch.chain_to_rxq, common_phv.chain_to_rxq); \
    modify_field(common_global_scratch.chain_doorbell_no_sched, common_phv.chain_doorbell_no_sched); \
    modify_field(common_global_scratch.desc_sem_pindex_full, common_phv.desc_sem_pindex_full); \
    modify_field(common_global_scratch.ppage_sem_pindex_full, common_phv.ppage_sem_pindex_full); \
    modify_field(common_global_scratch.mpage_sem_pindex_full, common_phv.mpage_sem_pindex_full); \
    modify_field(common_global_scratch.ppage_valid, common_phv.ppage_valid); \
    modify_field(common_global_scratch.mpage_valid, common_phv.mpage_valid); \
    modify_field(common_global_scratch.chain_ring_size_shift, common_phv.chain_ring_size_shift); \
    modify_field(common_global_scratch.chain_entry_size_shift, common_phv.chain_entry_size_shift); \
    modify_field(common_global_scratch.chain_ring_index_select, common_phv.chain_ring_index_select); \
    modify_field(common_global_scratch.chain_lif, common_phv.chain_lif); \
    modify_field(common_global_scratch.chain_qtype, common_phv.chain_qtype); \
    modify_field(common_global_scratch.chain_qid, common_phv.chain_qid); \
    modify_field(common_global_scratch.desc_valid_bit_req, common_phv.desc_valid_bit_req); \
    modify_field(common_global_scratch.redir_pipeline_lpbk_enable, common_phv.redir_pipeline_lpbk_enable); \
    modify_field(common_global_scratch.redir_span_instance, common_phv.redir_span_instance); \



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
        
        /*
         * Next service chain TxQ info, if any, for redirection type (b):
         * packet descriptors should be enqueued to the given TxQ
         * and the corresponding doorbell rung.
         */     
        chain_txq_base                  : HBM_ADDRESS_WIDTH;
        chain_txq_ring_indices_addr     : HBM_ADDRESS_WIDTH;
        
        chain_txq_qid                   : 32;
        chain_txq_lif                   : 16;
        chain_txq_qtype                 : 8;
        chain_txq_ring_size_shift       : 8;
        chain_txq_entry_size_shift      : 8;
        chain_txq_ring_index_select     : 8;
        
        chain_rxq_ring_size_shift       : 8;
        chain_rxq_entry_size_shift      : 8;
        chain_rxq_ring_index_select     : 8;
        
        chain_txq_doorbell_no_sched     : 8;    // set to update DB but skip scheduler bit
        desc_valid_bit_req              : 8;
        redir_pipeline_lpbk_enable      : 8;    // redir pipeline loopback indicator
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


/*
 * Global PHV definitions
 */
header_type common_global_phv_t {
    fields {
        // p4plus_common_global_t (max is GLOBAL_WIDTH or 128)
        chain_ring_base                 : HBM_ADDRESS_WIDTH;
        chain_to_rxq                    : 1;
        chain_doorbell_no_sched         : 1;
        desc_sem_pindex_full            : 1;
        ppage_sem_pindex_full           : 1;
        mpage_sem_pindex_full           : 1;
        ppage_valid                     : 1;
        mpage_valid                     : 1;
        desc_valid_bit_req              : 1;
        packet_len                      : 16;
        chain_lif                       : 11;
        chain_qtype                     : 3;
        chain_qid                       : 24;
        chain_ring_size_shift           : 5;
        chain_entry_size_shift          : 5;
        chain_ring_index_select         : 3;
        redir_pipeline_lpbk_enable      : 1;
        redir_span_instance             : 1;
    }
}

header_type dma_phv_pad_448_t {
    fields {
        dma_pad                         : 448;    
    }    
}

header_type dma_phv_pad_64_t {
    fields {
        dma_pad                         : 64;
    }    
}
    
header_type dma_phv_pad_256_t {
    fields {
        dma_pad                         : 256;
    }    
}

/*
 * to_stage PHV definitions
 */

header_type to_stage_5_phv_t {
    fields {
        // (max 128 bits)
        chain_ring_indices_addr         : HBM_ADDRESS_WIDTH;
    }
}

header_type to_stage_6_phv_t {
    fields {
        // (max 128 bits)
        desc                            : 32;
        ppage                           : 32;
        mpage                           : 32;
    }
}

header_type to_stage_7_phv_t {
    fields {
        // (max 128 bits)
        desc                            : 32;
        ppage                           : 32;
        mpage                           : 32;
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
metadata arq_rx_pi_d_t                  qidxr_chain_d;


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
metadata dma_phv_pad_64_t               dma_phv_pad_64;

@pragma dont_trim
metadata pkt_descr_t                    aol; 

@pragma dont_trim
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

@pragma dont_trim
metadata dma_phv_pad_256_t              dma_pad_256;


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
                     chain_rxq_base, chain_rxq_ring_indices_addr,
                     chain_rxq_ring_size_shift, chain_rxq_entry_size_shift,
                     chain_rxq_ring_index_select,
                     chain_txq_base, chain_txq_ring_indices_addr,
                     chain_txq_ring_size_shift, chain_txq_entry_size_shift,
                     chain_txq_lif, chain_txq_qtype, chain_txq_qid,
                     chain_txq_ring_index_select,
                     chain_txq_doorbell_no_sched, 
                     desc_valid_bit_req,
                     redir_pipeline_lpbk_enable) {
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
    modify_field(rawr_scratch_app.flow_hash, rawr_app_header.flow_hash);
    
    // packet_len is length of packet plus sizeof(p4_to_p4plus_cpu_pkt_t)
    // as set by P4 apps.p4
    modify_field(rawr_scratch_app.packet_len, rawr_app_header.packet_len);

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
    modify_field(rawrcb_d.chain_txq_doorbell_no_sched, chain_txq_doorbell_no_sched);
    modify_field(rawrcb_d.desc_valid_bit_req, desc_valid_bit_req);
    modify_field(rawrcb_d.redir_pipeline_lpbk_enable, redir_pipeline_lpbk_enable);
}

/*
 * Stage 1 table 0 action
 */
action desc_pindex_post_update(pindex, pindex_full) {
    modify_field(sem_desc_d.pindex, pindex);
    modify_field(sem_desc_d.pindex_full, pindex_full);
}

/*
 * Stage 1 table 1 action
 */
action ppage_pindex_post_update(pindex, pindex_full) {
    modify_field(sem_ppage_d.pindex, pindex);
    modify_field(sem_ppage_d.pindex_full, pindex_full);
}

/*
 * Stage 1 table 2 action
 */
action mpage_pindex_post_update(pindex, pindex_full) {
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
action chain_qidxr_stage_advance_a(void) {
}

/*
 * Stage 4 table 0 action
 */
action chain_qidxr_stage_advance_b(void) {
}

/*
 * Stage 5 table 0 action
 */
action chain_pindex_pre_alloc(void) {

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
action cleanup_discard(void) {
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
action chain_txq_pindex_post_read(pi_curr, ci_curr) {
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
 * Stage 6 table 2 action
 */
action chain_qidxr_pindex_post_read(pi_0, pi_1, pi_2) {
    // k + i for stage 6 table 2

    // from to_stage 6

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 6 table 2
    modify_field(qidxr_chain_d.pi_0, pi_0);
    modify_field(qidxr_chain_d.pi_1, pi_1);
    modify_field(qidxr_chain_d.pi_2, pi_2);
}


/*
 * Stage 6 table 3 action
 */
action chain_xfer(void) {
    // k + i for stage 6

    // from to_stage 6
    modify_field(to_s6_scratch.desc,  to_s6.desc);
    modify_field(to_s6_scratch.ppage, to_s6.ppage);
    modify_field(to_s6_scratch.mpage, to_s6.mpage);

    // from ki global
    GENERATE_GLOBAL_K
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


