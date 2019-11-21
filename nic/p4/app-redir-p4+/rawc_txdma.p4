/*********************************************************************************
 * L7 Raw Chain (to next service TxDMA or to P4)
 *********************************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"
#include "../cpu-p4+/cpu_rxtx_common.p4"
#include "app_redir_defines.h"

/*
 * L7 Raw Chain stage 0
 */
#define tx_table_s0_t0_action   rawc_tx_start


/*
 * Table names
 */
#define tx_table_s0_t0          s0_tbl
#define tx_table_s1_t0          s1_tbl
#define tx_table_s2_t0          s2_tbl
#define tx_table_s1_t1          s1_tbl1
#define tx_table_s2_t1          s2_tbl1
#define tx_table_s3_t1          s3_tbl1
#define tx_table_s4_t1          s4_tbl1
#define tx_table_s7_t2          s7_tbl2

/*
 * L7 Raw Chain stage 1
 */
#define tx_table_s1_t0_action   consume
#define tx_table_s1_t1_action   cb_extra_read

/*
 * L7 Raw Chain stage 2
 */
#define tx_table_s2_t0_action   pkt_prep
#define tx_table_s2_t1_action   desc_enqueue

/*
 * L7 Raw Chain stage 3
 */
#define tx_table_s3_t1_action   pkt_post

/*
 * L7 Raw Chain stage 4
 */
#define tx_table_s4_t1_action   desc_free

/*
 * L7 Raw Chain stage 7
 */
#define tx_table_s7_t2_action   metrics0_commit


#include "../common-p4+/common_txdma.p4"


/*
 * D-vectors
 */

/*
 * d for stage 0: 
 *    This is Raw Chain CB which is also its qstate. The relevant data
 *    contain info used for enqueuing packets to the next service in the chain
 *    or to P4. In other words, the data includes a pointer to the next service's
 *    TxQ ring which, if NULL, implies injecting to P4.
 */
header_type rawccb_t {
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
        rawccb_deactivate               : 8;  // must be first in CB after header rings
        pad                             : 8;
        rawccb_flags                    : 16; // DOL flags and others
        my_txq_base                     : 64;
        
        /*
         * Next service chain TxQ info which, if chain_txq_base is NULL,
         * implies that arrival buffers should be DMA'ed to P4. Otherwise,
         * the packet descriptor should be enqueued to the given TxQ
         * and the corresponding doorbell rung.
         */     
        chain_txq_base                  : 64;
        chain_txq_ring_indices_addr     : 64;
        chain_txq_ring_size_shift       : 8;
        chain_txq_entry_size_shift      : 8;
        chain_txq_qtype                 : 8;
        chain_txq_ring                  : 8;
        chain_txq_qid                   : 32;
        chain_txq_lif                   : 16;
        
        my_txq_ring_size_shift          : 8;
        my_txq_entry_size_shift         : 8;
        cpu_id                          : 8;
        
        /*
         * Sentinel to indicate all bytes in CB have been written and P4+ code
         * can start normal processing.
         */
        rawccb_activate                 : 8; // must be last in CB
    }
}

header_type rawccb_extra_t {
    fields {
        ascq_base                       : 64;
        ascq_sem_inf_addr               : 64;
    }
}

header_type rawccb_metrics0_t {
    fields {

        // CAUTION: order of fields must match rawc_kivec9_t
        chain_pkts                     	: 64;
        cb_not_ready_discards           : 64;
        qstate_cfg_discards             : 64;
        aol_error_discards              : 64;
        my_txq_empty_discards           : 64;
        txq_full_discards               : 64;
        pkt_free_errors                 : 64;
    }
}

header_type txq_desc_d_t {
    fields {
        desc                            : 64;
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
header_type rawc_kivec0_t {
    fields {
        chain_txq_ring_size_shift       : 8;
        chain_txq_entry_size_shift      : 8;
        rawccb_flags                    : 16;
        qstate_addr                     : 40;
        
        do_cleanup_discard              : 1;
        next_service_chain_action       : 1;
        pkt_freeq_not_cfg               : 1;
    }
}

#define RAWC_KIVEC0_USE(scratch, kivec)	\
    modify_field(scratch.chain_txq_ring_size_shift, kivec.chain_txq_ring_size_shift); \
    modify_field(scratch.chain_txq_entry_size_shift, kivec.chain_txq_entry_size_shift); \
    modify_field(scratch.rawccb_flags, kivec.rawccb_flags); \
    modify_field(scratch.qstate_addr, kivec.qstate_addr); \
    modify_field(scratch.do_cleanup_discard, kivec.do_cleanup_discard); \
    modify_field(scratch.next_service_chain_action, kivec.next_service_chain_action); \
    modify_field(scratch.pkt_freeq_not_cfg, kivec.pkt_freeq_not_cfg); \

/*
 * kivec1: header union with to_stage1 (128 bits max)
 */
header_type rawc_kivec1_t {
    fields {
        chain_txq_ring_indices_addr     : 64;
    }
}

#define RAWC_KIVEC1_USE(scratch, kivec)	\
    modify_field(scratch.chain_txq_ring_indices_addr, kivec.chain_txq_ring_indices_addr); \
    
/*
 * kivec2: header union with to_stage2 (128 bits max)
 */
header_type rawc_kivec2_t {
    fields {
        chain_txq_base                  : 64;
        chain_txq_lif                   : 11;
        chain_txq_qtype                 : 3;
        chain_txq_qid                   : 24;
        chain_txq_ring                  : 3;
    }
}

#define RAWC_KIVEC2_USE(scratch, kivec)	\
    modify_field(scratch.chain_txq_base, kivec.chain_txq_base); \
    modify_field(scratch.chain_txq_lif, kivec.chain_txq_lif); \
    modify_field(scratch.chain_txq_qtype, kivec.chain_txq_qtype); \
    modify_field(scratch.chain_txq_qid, kivec.chain_txq_qid); \
    modify_field(scratch.chain_txq_ring, kivec.chain_txq_ring); \
    
/*
 * kivec3: header union with stage_2_stage for table 1 (160 bits max)
 */
header_type rawc_kivec3_t {
    fields {
        desc                            : 64;
        ascq_sem_inf_addr               : 64;
        last_mem2pkt_ptr                : 32;
    }
}

#define RAWC_KIVEC3_USE(scratch, kivec)	\
    modify_field(scratch.desc, kivec.desc); \
    modify_field(scratch.ascq_sem_inf_addr, kivec.ascq_sem_inf_addr); \
    modify_field(scratch.last_mem2pkt_ptr, kivec.last_mem2pkt_ptr); \

/*
 * kivec4: header union with to_stage4 (128 bits max)
 */
header_type rawc_kivec4_t {
    fields {
        ascq_base                       : 64;
    }
}

#define RAWC_KIVEC4_USE(scratch, kivec)	\
    modify_field(scratch.ascq_base, kivec.ascq_base); \

/*
 * kivec9: header union with stage_2_stage for table 2 (160 bits max)
 */
header_type rawc_kivec9_t {
    fields {
        // CAUTION: order of fields must match rawccb_metrics0_t
        metrics0_start                  : 1;
        chain_pkts                      : 1;
        cb_not_ready_discards           : 1;
        qstate_cfg_discards             : 1;
        aol_error_discards              : 1;
        my_txq_empty_discards           : 1;
        txq_full_discards               : 1;
        pkt_free_errors                 : 1;
        metrics0_end                    : 1;
    }
}

#define RAWC_KIVEC9_USE(scratch, kivec)	\
    modify_field(scratch.metrics0_start, kivec.metrics0_start); \
    modify_field(scratch.chain_pkts, kivec.chain_pkts); \
    modify_field(scratch.cb_not_ready_discards, kivec.cb_not_ready_discards); \
    modify_field(scratch.qstate_cfg_discards, kivec.qstate_cfg_discards); \
    modify_field(scratch.aol_error_discards, kivec.aol_error_discards); \
    modify_field(scratch.my_txq_empty_discards, kivec.my_txq_empty_discards); \
    modify_field(scratch.txq_full_discards, kivec.txq_full_discards); \
    modify_field(scratch.pkt_free_errors, kivec.pkt_free_errors); \
    modify_field(scratch.metrics0_end, kivec.metrics0_end); \
    
/*
 * Header unions for d-vector
 */
@pragma scratch_metadata
metadata rawccb_t                       rawccb_d;

@pragma scratch_metadata
metadata rawccb_extra_t                 rawccb_extra_d;

@pragma scratch_metadata
metadata txq_desc_d_t                   my_txq_desc_d;

@pragma scratch_metadata
metadata pkt_descr_aol_t                txq_desc_aol_d;

@pragma scratch_metadata
metadata chain_txq_ring_indices_d_t     chain_txq_ring_indices_d;

@pragma scratch_metadata
metadata cpu_to_p4plus_header_t         cpu_to_p4plus_header_d;

@pragma scratch_metadata
metadata sem_ascq_d_t                   sem_ascq_d;

@pragma scratch_metadata
metadata rawccb_metrics0_t              rawccb_metrics0;

/*
 * Header unions for PHV layout
 */
@pragma pa_header_union ingress         common_global
metadata rawc_kivec0_t                  rawc_kivec0;
@pragma scratch_metadata
metadata rawc_kivec0_t                  rawc_kivec0_scratch;

@pragma pa_header_union ingress         app_header
metadata p4plus_to_p4_header_t          rawc_app_header;
@pragma scratch_metadata
metadata p4plus_to_p4_header_t          rawc_scratch_app;

@pragma pa_header_union ingress         to_stage_1
metadata rawc_kivec1_t                  rawc_kivec1;
@pragma scratch_metadata
metadata rawc_kivec1_t                  rawc_kivec1_scratch;

@pragma pa_header_union ingress         to_stage_2
metadata rawc_kivec2_t                  rawc_kivec2;
@pragma scratch_metadata
metadata rawc_kivec2_t                  rawc_kivec2_scratch;

@pragma pa_header_union ingress         common_t1_s2s
metadata rawc_kivec3_t                  rawc_kivec3;
@pragma scratch_metadata
metadata rawc_kivec3_t                  rawc_kivec3_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         to_stage_4
metadata rawc_kivec4_t                  rawc_kivec4;
@pragma scratch_metadata
metadata rawc_kivec4_t                  rawc_kivec4_scratch;

@pragma dont_trim
@pragma pa_header_union ingress         common_t2_s2s
metadata rawc_kivec9_t                  rawc_kivec9;
@pragma scratch_metadata
metadata rawc_kivec9_t                  rawc_kivec9_scratch;

/*
 * PHV following k (for app DMA etc.)
 */
@pragma dont_trim
metadata ring_entry_t                   ring_entry; 

@pragma dont_trim
metadata doorbell_data_raw_t            chain_txq_db_data; 

/*
 * DMA descriptors for injecting packet data to P4
 */
@pragma dont_trim
@pragma pa_align 512
metadata dma_cmd_phv2pkt_t              dma_intr_global;
@pragma dont_trim
metadata dma_cmd_phv2pkt_t              dma_intr_txdma;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t              dma_pkt0;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t              dma_pkt1;
@pragma dont_trim
metadata dma_cmd_mem2pkt_t              dma_pkt2;

/*
 * DMA descriptors for enqueuing to next service TxQ
 */
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
 *  rawc_tx_start : Initial stage
 *****************************************************************************/
action rawc_tx_start(rsvd, cosA, cosB, cos_sel, 
                     eval_last, host, total, pid,
                     pi_0, ci_0,
                     rawccb_deactivate, pad, rawccb_flags,
                     my_txq_base, chain_txq_base, chain_txq_ring_indices_addr,
                     chain_txq_ring_size_shift, chain_txq_entry_size_shift,
		     chain_txq_qtype, chain_txq_ring, chain_txq_qid,
                     chain_txq_lif, my_txq_ring_size_shift,
                     my_txq_entry_size_shift, cpu_id, rawccb_activate) {

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_txdma_intr_scratch.qid, p4_txdma_intr.qid);
    modify_field(p4_txdma_intr_scratch.qtype, p4_txdma_intr.qtype);
    modify_field(p4_txdma_intr_scratch.qstate_addr, p4_txdma_intr.qstate_addr);

    // from p4plus_to_p4_header_t
    modify_field(rawc_scratch_app.p4plus_app_id, rawc_app_header.p4plus_app_id);
    modify_field(rawc_scratch_app.flags, rawc_app_header.flags);
    modify_field(rawc_scratch_app.ip_id_delta, rawc_app_header.ip_id_delta);
    modify_field(rawc_scratch_app.tcp_seq_delta, rawc_app_header.tcp_seq_delta);
    modify_field(rawc_scratch_app.vlan_tag, rawc_app_header.vlan_tag);
    
    //modify_field(rawccb_d.pc, pc);
    modify_field(rawccb_d.rsvd, rsvd);
    modify_field(rawccb_d.cosA, cosA);
    modify_field(rawccb_d.cosB, cosB);
    modify_field(rawccb_d.cos_sel, cos_sel);
    modify_field(rawccb_d.eval_last, eval_last);
    modify_field(rawccb_d.host, host);
    modify_field(rawccb_d.total, total);
    modify_field(rawccb_d.pid, pid);
    modify_field(rawccb_d.pi_0, pi_0);
    modify_field(rawccb_d.ci_0, ci_0);
    
    modify_field(rawccb_d.rawccb_deactivate, rawccb_deactivate);
    modify_field(rawccb_d.pad, pad);
    modify_field(rawccb_d.rawccb_flags, rawccb_flags);
    modify_field(rawccb_d.my_txq_base, my_txq_base);
    modify_field(rawccb_d.chain_txq_base, chain_txq_base);
    modify_field(rawccb_d.chain_txq_ring_indices_addr, chain_txq_ring_indices_addr);
    modify_field(rawccb_d.chain_txq_ring_size_shift, chain_txq_ring_size_shift);
    modify_field(rawccb_d.chain_txq_entry_size_shift, chain_txq_entry_size_shift);
    modify_field(rawccb_d.chain_txq_qtype, chain_txq_qtype);
    modify_field(rawccb_d.chain_txq_ring, chain_txq_ring);
    modify_field(rawccb_d.chain_txq_qid, chain_txq_qid);
    modify_field(rawccb_d.chain_txq_lif, chain_txq_lif);
    modify_field(rawccb_d.my_txq_ring_size_shift, my_txq_ring_size_shift);
    modify_field(rawccb_d.my_txq_entry_size_shift, my_txq_entry_size_shift);
    modify_field(rawccb_d.cpu_id, cpu_id);
    modify_field(rawccb_d.rawccb_activate, rawccb_activate);
}


/*****************************************************************************
 *  cb_extra_read : Read extra portion of rawccb
 *****************************************************************************/
action cb_extra_read(ascq_base, ascq_sem_inf_addr) {

    modify_field(rawccb_extra_d.ascq_base, ascq_base);
    modify_field(rawccb_extra_d.ascq_sem_inf_addr, ascq_sem_inf_addr);
}


/*****************************************************************************
 *  consume : Consume descriptor at my_txq current cindex
 *****************************************************************************/
action consume(desc) {

    RAWC_KIVEC0_USE(rawc_kivec0_scratch, rawc_kivec0)
    RAWC_KIVEC1_USE(rawc_kivec1_scratch, rawc_kivec1)
    
    // d for stage and table
    modify_field(my_txq_desc_d.desc, desc);
}


/*****************************************************************************
 *  desc_enqueue : Enqueue packet descriptor for chaining to a next service TxQ.
 *****************************************************************************/
action desc_enqueue(pi_curr, ci_curr) {

    RAWC_KIVEC0_USE(rawc_kivec0_scratch, rawc_kivec0)
    RAWC_KIVEC2_USE(rawc_kivec2_scratch, rawc_kivec2)
    RAWC_KIVEC3_USE(rawc_kivec3_scratch, rawc_kivec3)
    
    // d for stage and table
    modify_field(chain_txq_ring_indices_d.pi_curr, pi_curr);
    modify_field(chain_txq_ring_indices_d.ci_curr, ci_curr);
}


/*****************************************************************************
 *  pkt_prep : Prepare packet descriptor for P4 transmission, stripping CPU/P4+
 *             headers as necessary.
 *****************************************************************************/
action pkt_prep(A0, O0, L0,
                A1, O1, L1,
                A2, O2, L2) {

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


/*****************************************************************************
 *  pkt_post : Initiate mem2pkt DMA
 *****************************************************************************/
action pkt_post(flags, src_lif, hw_vlan_id, l2_offset) {

    RAWC_KIVEC0_USE(rawc_kivec0_scratch, rawc_kivec0)
    RAWC_KIVEC3_USE(rawc_kivec3_scratch, rawc_kivec3)
    
    // d for stage and table
    modify_field(cpu_to_p4plus_header_d.flags, flags);
    modify_field(cpu_to_p4plus_header_d.src_lif, src_lif);
    modify_field(cpu_to_p4plus_header_d.hw_vlan_id, hw_vlan_id);
    modify_field(cpu_to_p4plus_header_d.l2_offset, l2_offset);
}


/*****************************************************************************
 *  desc_free : Enqueue packet descriptor to ASCQ for freeing
 *****************************************************************************/
action desc_free(ascq_pindex, ascq_full) {

    RAWC_KIVEC0_USE(rawc_kivec0_scratch, rawc_kivec0)
    RAWC_KIVEC3_USE(rawc_kivec3_scratch, rawc_kivec3)
    RAWC_KIVEC4_USE(rawc_kivec4_scratch, rawc_kivec4)
    
    modify_field(sem_ascq_d.ascq_pindex, ascq_pindex);
    modify_field(sem_ascq_d.ascq_full, ascq_full);
}

/*****************************************************************************
 *  metrics0_commit : Update and commit metrics0 to qstate.
 *****************************************************************************/
@pragma little_endian chain_pkts cb_not_ready_discards qstate_cfg_discards aol_error_discards my_txq_empty_discards txq_full_discards pkt_free_errors
action metrics0_commit(chain_pkts, cb_not_ready_discards,
                       qstate_cfg_discards, aol_error_discards,
		       my_txq_empty_discards, txq_full_discards,
		       pkt_free_errors) {
			   
  RAWC_KIVEC9_USE(rawc_kivec9_scratch, rawc_kivec9)
  
  modify_field(rawccb_metrics0.chain_pkts, chain_pkts);
  modify_field(rawccb_metrics0.cb_not_ready_discards, cb_not_ready_discards);
  modify_field(rawccb_metrics0.qstate_cfg_discards, qstate_cfg_discards);
  modify_field(rawccb_metrics0.aol_error_discards, aol_error_discards);
  modify_field(rawccb_metrics0.my_txq_empty_discards, my_txq_empty_discards);
  modify_field(rawccb_metrics0.txq_full_discards, txq_full_discards);
  modify_field(rawccb_metrics0.pkt_free_errors, pkt_free_errors);
}

