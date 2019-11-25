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
#define rx_table_s0_t0          s0_tbl
#define rx_table_s1_t0          s1_tbl
#define rx_table_s1_t1          s1_tbl1
#define rx_table_s2_t0          s2_tbl
#define rx_table_s4_t0          s4_tbl
#define rx_table_s5_t0          s5_tbl
#define rx_table_s5_t1          s5_tbl1
#define rx_table_s6_t0          s6_tbl
#define rx_table_s6_t3          s6_tbl3
#define rx_table_s7_t0          s7_tbl

#define rx_table_s7_t2          s7_tbl2

/*
 * L7 Raw Redirect stage 1
 */
#define rx_table_s1_t0_action   ppage_pindex_post_update 
#define rx_table_s1_t1_action   cb_extra_read

/*
 * L7 Raw Redirect stage 2
 */
#define rx_table_s2_t0_action   ppage_post_alloc

/*
 * Stage 3 is for hash defined in common rxdma
 * WARNING: Toeplitz structures are overlaid over multiple to_stateN
 * structures in the PHV so Rxdma programs which depend on Toeplitz
 * (such as this program) must use only tN_s2s and global keys.
 */

/*
 * L7 Raw Redirect stage 4
 */
#define rx_table_s4_t0_action   chain_pindex_pre_alloc

/*
 * L7 Raw Redirect stage 5
 */
#define rx_table_s5_t0_action   cleanup_discard

#define rx_table_s5_t1_action   chain_xfer
#define rx_table_s5_t1_action1  txq_post_read
#define rx_table_s5_t1_action2  post_update

/*
 * L7 Raw Redirect stage 6
 */
#define rx_table_s6_t3_action   ppage_free

/*
 * L7 Raw Redirect stage 7
 */
#define rx_table_s7_t2_action   metrics0_commit


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
        cpu_id                          : 8;
        
        /*
         * Sentinel to indicate all bytes in CB have been written and P4+ code
         * can start normal processing.
         */
        rawrcb_activate                 : 8; // must be last in CB
    }
}

header_type rawrcb_extra_t {
    fields {
        ascq_base                       : 64;
        ascq_sem_inf_addr               : 64;
    }
}

// Qstate metrics.
header_type rawrcb_metrics0_t {
  fields {

    // CAUTION: order of fields must match rawr_kivec9_t
    redir_pkts                     	: 64;
    cb_not_ready_discards               : 64;
    qstate_cfg_discards                 : 64;
    pkt_len_discards                    : 64;
    rxq_full_discards                   : 64;
    txq_full_discards                   : 64;
    pkt_alloc_errors                    : 64;
    pkt_free_errors                     : 64;
  }
}

header_type sem_ppage_d_t {
    fields {
        page_pindex                     : 32;
        page_pindex_full                : 8;
    }
}

header_type alloc_ppage_d_t {
    fields {
        page                            : 64;
        pad                             : 448;
    }
}

header_type chain_txq_ring_indices_d_t {
    fields {
        CAPRI_QSTATE_HEADER_RING(curr)
        pad                             : 480;
    }
}

header_type sem_ascq_d_t {
    fields {
        ascq_pindex      : 32;
        ascq_full        : 8;
    }
} 

/*
 * kivec0: header union with global (128 bits max)
 */
header_type rawr_kivec0_t {
    fields {
        chain_ring_index_select         : 3;
        chain_ring_size_shift           : 5;
        rawrcb_flags                    : 16;
        packet_len                      : 16;
        qstate_addr                     : 34;
        ppage                           : 40;
        chain_to_rxq                    : 1;
        redir_span_instance             : 1;
        ppage_sem_pindex_full           : 1;
        do_cleanup_discard              : 1;
        pkt_freeq_not_cfg               : 1;
    }
}

#define RAWR_KIVEC0_USE(scratch, kivec)	\
    modify_field(scratch.chain_ring_index_select, kivec.chain_ring_index_select); \
    modify_field(scratch.chain_ring_size_shift, kivec.chain_ring_size_shift); \
    modify_field(scratch.rawrcb_flags, kivec.rawrcb_flags); \
    modify_field(scratch.packet_len, kivec.packet_len); \
    modify_field(scratch.qstate_addr, kivec.qstate_addr); \
    modify_field(scratch.ppage, kivec.ppage); \
    modify_field(scratch.chain_to_rxq, kivec.chain_to_rxq); \
    modify_field(scratch.redir_span_instance, kivec.redir_span_instance); \
    modify_field(scratch.ppage_sem_pindex_full, kivec.ppage_sem_pindex_full); \
    modify_field(scratch.do_cleanup_discard, kivec.do_cleanup_discard); \
    modify_field(scratch.pkt_freeq_not_cfg, kivec.pkt_freeq_not_cfg); \

/*
 * kivec1: header union with stage_2_stage for table 0 (160 bits max)
 */
header_type rawr_kivec1_t {
    fields {
        chain_ring_indices_addr         : 64;
        ascq_sem_inf_addr               : 40;
    }
}

#define RAWR_KIVEC1_USE(scratch, kivec)	\
    modify_field(scratch.chain_ring_indices_addr, kivec.chain_ring_indices_addr); \
    modify_field(scratch.ascq_sem_inf_addr, kivec.ascq_sem_inf_addr); \
    
/*
 * kivec1: header union with stage_2_stage for table 1 (160 bits max)
 */
header_type rawr_kivec2_t {
    fields {
        chain_lif                       : 11;
        chain_qtype                     : 3;
        chain_qid                       : 24;
        chain_ring_base                 : 64;
        chain_entry_size_shift          : 5;
    }
}

#define RAWR_KIVEC2_USE(scratch, kivec)	\
    modify_field(scratch.chain_lif, kivec.chain_lif); \
    modify_field(scratch.chain_qtype, kivec.chain_qtype); \
    modify_field(scratch.chain_qid, kivec.chain_qid); \
    modify_field(scratch.chain_ring_base, kivec.chain_ring_base); \
    modify_field(scratch.chain_entry_size_shift, kivec.chain_entry_size_shift); \
    
/*
 * kivec1: header union with stage_2_stage for table 3 (160 bits max)
 */
header_type rawr_kivec3_t {
    fields {
        ascq_base                       : 64;
    }
}

#define RAWR_KIVEC3_USE(scratch, kivec)	\
    modify_field(scratch.ascq_base, kivec.ascq_base); \
    
/*
 * kivec9: header union with stage_2_stage for table 2 (160 bits max)
 */
header_type rawr_kivec9_t {
    fields {
    
        // CAUTION: order of fields must match rawrcb_metrics0_t
        metrics0_start                  : 1;
        redir_pkts                      : 1;
        cb_not_ready_discards           : 1;
        qstate_cfg_discards             : 1;
        pkt_len_discards                : 1;
        rxq_full_discards               : 1;
        txq_full_discards               : 1;
        pkt_alloc_errors                : 1;
        pkt_free_errors                 : 1;
        metrics0_end                    : 1;
    }
}

#define RAWR_KIVEC9_USE(scratch, kivec)	\
    modify_field(scratch.metrics0_start, kivec.metrics0_start); \
    modify_field(scratch.redir_pkts, kivec.redir_pkts); \
    modify_field(scratch.cb_not_ready_discards, kivec.cb_not_ready_discards); \
    modify_field(scratch.qstate_cfg_discards, kivec.qstate_cfg_discards); \
    modify_field(scratch.pkt_len_discards, kivec.pkt_len_discards); \
    modify_field(scratch.rxq_full_discards, kivec.rxq_full_discards); \
    modify_field(scratch.txq_full_discards, kivec.txq_full_discards); \
    modify_field(scratch.pkt_alloc_errors, kivec.pkt_alloc_errors); \
    modify_field(scratch.pkt_free_errors, kivec.pkt_free_errors); \
    modify_field(scratch.metrics0_end, kivec.metrics0_end); \

/*
 * Header unions for d-vector
 */
@pragma scratch_metadata
metadata rawrcb_t                       rawrcb_d;

@pragma scratch_metadata
metadata rawrcb_extra_t                 rawrcb_extra_d;

@pragma scratch_metadata
metadata sem_ppage_d_t                  sem_ppage_d;

@pragma scratch_metadata
metadata alloc_ppage_d_t                alloc_ppage_d;

@pragma scratch_metadata
metadata chain_txq_ring_indices_d_t     chain_txq_ring_indices_d;

@pragma scratch_metadata
metadata arq_pi_d_t                     qidxr_chain_d;

@pragma scratch_metadata
metadata sem_ascq_d_t                   sem_ascq_d;

@pragma scratch_metadata
metadata rawrcb_metrics0_t              rawrcb_metrics0;

/*
 * Header unions for PHV layout
 */
@pragma pa_header_union ingress         common_global
metadata rawr_kivec0_t                  rawr_kivec0;
@pragma scratch_metadata
metadata rawr_kivec0_t                  rawr_kivec0_scratch;

@pragma pa_header_union ingress         app_header
metadata p4_to_p4plus_cpu_header_ext_t  rawr_app_header;
@pragma scratch_metadata
metadata p4_to_p4plus_cpu_header_ext_t  rawr_scratch_app;

@pragma pa_header_union ingress         common_t0_s2s
metadata rawr_kivec1_t                  rawr_kivec1;
@pragma scratch_metadata
metadata rawr_kivec1_t                  rawr_kivec1_scratch;

@pragma pa_header_union ingress         common_t1_s2s
metadata rawr_kivec2_t                  rawr_kivec2;
@pragma scratch_metadata
metadata rawr_kivec2_t                  rawr_kivec2_scratch;

@pragma pa_header_union ingress         common_t3_s2s
metadata rawr_kivec3_t                  rawr_kivec3;
@pragma scratch_metadata
metadata rawr_kivec3_t                  rawr_kivec3_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         common_t2_s2s
metadata rawr_kivec9_t                  rawr_kivec9;
@pragma scratch_metadata
metadata rawr_kivec9_t                  rawr_kivec9_scratch;

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

/*****************************************************************************
 *  rawr_rx_start : Initial stage
 *****************************************************************************/
action rawr_rx_start(rsvd, cosA, cosB, cos_sel, 
                     eval_last, host, total, pid,
                     rawrcb_deactivate, chain_rxq_base,
                     chain_rxq_ring_indices_addr, chain_rxq_ring_size_shift,
                     chain_rxq_entry_size_shift, chain_rxq_ring_index_select,
                     chain_txq_base, chain_txq_ring_indices_addr,
                     chain_txq_ring_size_shift, chain_txq_entry_size_shift,
                     chain_txq_lif, chain_txq_qtype, chain_txq_qid,
                     chain_txq_ring_index_select,
                     rawrcb_flags, cpu_id, rawrcb_activate) {
                     
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
    modify_field(rawr_scratch_app.packet_type, rawr_app_header.packet_type);
    modify_field(rawr_scratch_app.ip_sa, rawr_app_header.ip_sa);
    modify_field(rawr_scratch_app.ip_da1, rawr_app_header.ip_da1);
    modify_field(rawr_scratch_app.ip_da2, rawr_app_header.ip_da2);

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
    modify_field(rawrcb_d.cpu_id, cpu_id);
    modify_field(rawrcb_d.rawrcb_activate, rawrcb_activate);
}

/*****************************************************************************
 *  cb_extra_read : Read extra portion of rawccb
 *****************************************************************************/
action cb_extra_read(ascq_base, ascq_sem_inf_addr) {

    modify_field(rawrcb_extra_d.ascq_base, ascq_base);
    modify_field(rawrcb_extra_d.ascq_sem_inf_addr, ascq_sem_inf_addr);
}


/*****************************************************************************
 *  ppage_pindex_post_update : Obtain page index from a descriptor-page
 *                             semaphore.
 *****************************************************************************/
action ppage_pindex_post_update(page_pindex, page_pindex_full) {
    
    modify_field(sem_ppage_d.page_pindex, page_pindex);
    modify_field(sem_ppage_d.page_pindex_full, page_pindex_full);
}

/*****************************************************************************
 *  ppage_post_alloc : Obtain page address from page index fetched in previous
 *                     stage.
 *****************************************************************************/
action ppage_post_alloc(page, pad) {

    RAWR_KIVEC0_USE(rawr_kivec0_scratch, rawr_kivec0)

    modify_field(alloc_ppage_d.page, page);
    modify_field(alloc_ppage_d.pad, pad);
}

/*****************************************************************************
 *  chain_pindex_pre_alloc : Intermediate stage to launch chain queue pindex
 *                           allocation.
 *****************************************************************************/
action chain_pindex_pre_alloc() {

    RAWR_KIVEC0_USE(rawr_kivec0_scratch, rawr_kivec0)
    RAWR_KIVEC1_USE(rawr_kivec1_scratch, rawr_kivec1)
}

/*****************************************************************************
 *  post_update : 
 *****************************************************************************/
action post_update(ARQ_PI_PARAMS) {

    RAWR_KIVEC0_USE(rawr_kivec0_scratch, rawr_kivec0)
    RAWR_KIVEC2_USE(rawr_kivec2_scratch, rawr_kivec2)

    GENERATE_ARQ_PI_D(qidxr_chain_d)
}

/*****************************************************************************
 *  txq_post_read : 
 *****************************************************************************/
action txq_post_read(pi_curr, ci_curr) {

    RAWR_KIVEC0_USE(rawr_kivec0_scratch, rawr_kivec0)
    RAWR_KIVEC2_USE(rawr_kivec2_scratch, rawr_kivec2)

    modify_field(chain_txq_ring_indices_d.pi_curr, pi_curr);
    modify_field(chain_txq_ring_indices_d.ci_curr, ci_curr);
}


/*****************************************************************************
 *  cleanup_discard : Clean up and discard any allocated descriptor-page.
 *****************************************************************************/
action cleanup_discard() {

    RAWR_KIVEC0_USE(rawr_kivec0_scratch, rawr_kivec0)
    RAWR_KIVEC1_USE(rawr_kivec1_scratch, rawr_kivec1)
}


/*****************************************************************************
 *  chain_xfer : 
 *****************************************************************************/
action chain_xfer() {

    RAWR_KIVEC0_USE(rawr_kivec0_scratch, rawr_kivec0)
    RAWR_KIVEC2_USE(rawr_kivec2_scratch, rawr_kivec2)
}


/*****************************************************************************
 *  ppage_free: Enqueue ppage to ASCQ for freeing 
 *****************************************************************************/
action ppage_free(ascq_pindex, ascq_full) {
    
    RAWR_KIVEC0_USE(rawr_kivec0_scratch, rawr_kivec0)
    RAWR_KIVEC3_USE(rawr_kivec3_scratch, rawr_kivec3)

    modify_field(sem_ascq_d.ascq_pindex, ascq_pindex);
    modify_field(sem_ascq_d.ascq_full, ascq_full);
}


/*****************************************************************************
 *  metrics0_commit : Update and commit metrics0 to qstate.
 *****************************************************************************/
@pragma little_endian redir_pkts cb_not_ready_discards qstate_cfg_discards pkt_len_discards rxq_full_discards txq_full_discards pkt_alloc_errors pkt_free_errors
action metrics0_commit(redir_pkts, cb_not_ready_discards,
                       qstate_cfg_discards, pkt_len_discards,
		       rxq_full_discards, txq_full_discards,
		       pkt_alloc_errors, pkt_free_errors) {
			   
  RAWR_KIVEC9_USE(rawr_kivec9_scratch, rawr_kivec9)
  
  modify_field(rawrcb_metrics0.redir_pkts, redir_pkts);
  modify_field(rawrcb_metrics0.cb_not_ready_discards, cb_not_ready_discards);
  modify_field(rawrcb_metrics0.qstate_cfg_discards, qstate_cfg_discards);
  modify_field(rawrcb_metrics0.pkt_len_discards, pkt_len_discards);
  modify_field(rawrcb_metrics0.rxq_full_discards, rxq_full_discards);
  modify_field(rawrcb_metrics0.txq_full_discards, txq_full_discards);
  modify_field(rawrcb_metrics0.pkt_alloc_errors, pkt_alloc_errors);
  modify_field(rawrcb_metrics0.pkt_free_errors, pkt_free_errors);
}

