/*****************************************************************************
 *  seq_q_state_pop : Check the queue state and see if there's anything to be
 *                    popped. If so increment the working index and load the
 *                    queue entry.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s0_tbl_k k;
struct s0_tbl_seq_q_state_pop_d d;
struct phv_ p;

/*
 * Registers usage
 * CAUTION: r1 is also implicitly used by LOAD_TABLE1_FOR_ADDR_PC_IMM()
 */
#define r_ci                        r2  // my_txq onsumer index
#define r_pi                        r3  // my_txq producer index
#define r_qdesc_size                r4  // queue descriptor size
#define r_qdesc0                    r5  // desc0 pointer
#define r_qdesc1                    r6  // desc1 pointer
#define r_qstate_addr               r7  // for SEQ_PHV_STAT_TABLE2/3_COMMIT()

/*
 * Registers reuse, post descriptors loading and stat tables update/commit
 */
#define r_db_addr_scratch           r_qdesc0  // doorbell address
#define r_db_data_scratch           r_qdesc1  // doorbell data

/*
 * Registers reuse, abort phase
 */
#define r_seq_ring_size             r_qdesc_size
#define r_num_descs                 r_qdesc0

%%
   SEQ_METRICS_PARAMS()

storage_seq_q_state_pop_start:

   /*
    * Stage 0 table engine always fetches the qstate (in d-vector)
    * for processing by the stage 0 MPUs.
    * 
    * In addition, 
    * R1 = table lookup hash value
    * R2 = packet size
    * R3 = random number
    * R4 = current time
    * R5 = programmable table constant
    * R6 = phv timestamp
    * R7 = qstate ring_not_empty
    */
   //bbeq         r7[0], 0, drop_n_exit
   seq          c1, r7[0], r0
   bcf          [c1], drop_n_exit
   add		r_ci, r0, d.c_ndx       // delay slot
   bbeq         d.abort[0], 1, abort
   add          r_pi, r0, d.p_ndx       // delay slot     
   bbeq         d.enable[0], 0, drop_n_exit

   // Store fields needed in the K+I vector into the PHV
   phvwrpair	p.seq_kivec1_src_lif, STAGE0_KIVEC_LIF, \
        	p.seq_kivec1_src_qtype, STAGE0_KIVEC_QTYPE // delay slot
   phvwrpair	p.seq_kivec1_src_qid, STAGE0_KIVEC_QID, \
   	        p.seq_kivec1_src_qaddr, STAGE0_KIVEC_QADDR
   phvwr        p.seq_kivec6_src_qaddr, STAGE0_KIVEC_QADDR
   phvwr        p.seq_kivec8_src_qaddr, STAGE0_KIVEC_QADDR
   
   // Set the table and program address for the next stage to process
   // the popped entry
   add          r_qdesc_size, r0, d.entry_size
   sll          r_qdesc0, r_ci, r_qdesc_size
   add          r_qdesc0, r_qdesc0, d.base_addr

   LOAD_TABLE0_FOR_ADDR64_CONT(r_qdesc0,
                               STORAGE_DEFAULT_TBL_LOAD_SIZE,
                               d.next_pc[27:0])
   /*
    * Initiate launch of metrics tables update/commit here. Subsequent stages
    * launched from the same tables would be required to relaunch the
    * the affected metrics tables update.
    */
   SEQ_METRICS0_TABLE1_COMMIT(STAGE0_KIVEC_QADDR)
   SEQ_METRICS1_TABLE2_COMMIT(STAGE0_KIVEC_QADDR)
   SEQ_METRICS2_TABLE3_COMMIT(STAGE0_KIVEC_QADDR)

if0:                               
   bbeq         d.desc1_next_pc_valid[0], 0, endif0
   add          r_qdesc1, r_qdesc0, STORAGE_DEFAULT_TBL_LOAD_SIZE_BYTES // delay slot
   LOAD_TABLE1_FOR_ADDR64(r_qdesc1,
                          STORAGE_DEFAULT_TBL_LOAD_SIZE,
                          d.desc1_next_pc[27:0])
endif0:                          
   mincr        r_ci, d.num_entries, 1
   mincr        r_pi, d.num_entries, r0

   SEQ_METRICS_SET(descs_processed)
   
   /*
    * if new CI now == PI, clear scheduler bit
    */
   sne          c1, r_ci, r_pi
   nop.c1.e
   tblmincri.f  d.c_ndx, d.num_entries, 1       // delay slot

   QUEUE_DOORBELL_CLEAR_INLINE_e(r0,
                                 DOORBELL_SCHED_WR_EVAL,
                                 STAGE0_KIVEC_LIF,
                                 STAGE0_KIVEC_QTYPE,
                                 STAGE0_KIVEC_QID)
   
abort:

   /*
    * Discard all outstanding descriptors
    */
   tblwr        d.abort, r0
   tblwr.f      d.c_ndx, r_pi
   QUEUE_DOORBELL_CLEAR_INLINE(r0,
                               DOORBELL_SCHED_WR_EVAL,
                               STAGE0_KIVEC_LIF,
                               STAGE0_KIVEC_QTYPE,
                               STAGE0_KIVEC_QID)
   /*
    * Keep track of # of aborted descs
    */
   sll          r_seq_ring_size, 1, d.num_entries
   slt          c1, r_pi, r_ci
   add.c1       r_pi, r_pi, r_seq_ring_size
   sub          r_num_descs, r_pi, r_ci
   SEQ_METRICS_VAL_SET(descs_aborted, r_num_descs)
   SEQ_METRICS0_TABLE1_COMMIT(STAGE0_KIVEC_QADDR)
       
drop_n_exit:

   phvwr.e	p.p4_intr_global_drop, 1
   CLEAR_TABLE0                         // delay slot

