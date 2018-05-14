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
 */
#define r_ci                        r1  // my_txq onsumer index
#define r_pi                        r2  // my_txq producer index
#define r_qdesc_size                r3  // queue descriptor size
#define r_qdesc0                    r4  // desc0 pointer
#define r_qdesc1                    r5  // desc1 pointer
#define r_db_addr_scratch           r6  // doorbell address
#define r_db_data_scratch           r7  // doorbell data
%%

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
   seq          c1, r7[0:0], r0
   bcf          [c1], drop_n_exit
   add		r_ci, r0, d.c_ndx         // delay slot

   // Store fields needed in the K+I vector into the PHV
   phvwr        p.{seq_kivec0_dst_lif...seq_kivec0_dst_qaddr}, \
                d.{dst_lif...dst_qaddr}
                
   phvwrpair	p.seq_kivec1_src_lif, STAGE0_KIVEC_LIF, \
        	p.seq_kivec1_src_qtype, STAGE0_KIVEC_QTYPE
   phvwrpair	p.seq_kivec1_src_qid, STAGE0_KIVEC_QID, \
   	        p.seq_kivec1_src_qaddr, STAGE0_KIVEC_QADDR
   
   // Set the table and program address for the next stage to process
   // the popped entry
   add          r_qdesc_size, r0, d.entry_size
   sll          r_qdesc0, r_ci, r_qdesc_size
   add          r_qdesc0, r_qdesc0, d.base_addr

   bbeq         d.desc1_next_pc_valid, 0, load_table_default
   add          r_qdesc1, r_qdesc0, STORAGE_DEFAULT_TBL_LOAD_SIZE_BYTES // delay slot
   add          r_qdesc_size, r0, STORAGE_DEFAULT_TBL_LOAD_SIZE
   LOAD_TABLE1_FOR_ADDR64(r_qdesc1, r_qdesc_size, d.desc1_next_pc)

load_table_default:   
   LOAD_TABLE0_FOR_ADDR64_CONT(r_qdesc0, r_qdesc_size, d.next_pc)
   
   mincr        r_ci, d.num_entries, 1
   add          r_pi, r0, d.p_ndx
   mincr        r_pi, d.num_entries, r0

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
drop_n_exit:

   phvwr.e	p.p4_intr_global_drop, 1
   CLEAR_TABLE0 // delay slot
