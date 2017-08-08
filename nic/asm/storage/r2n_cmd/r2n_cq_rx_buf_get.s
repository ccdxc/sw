/**********************************************************
 *  Stage: Get the ROCE Rx buf to post
 **********************************************************/

#include "asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct r2n_cq_rx_buf_k k;
struct r2n_cq_rx_buf_get_d d;
struct phv_ p;

%%
   .param	r2n_rq_entry_push_start

r2n_cq_rx_buf_get_start:
   // Save the entire d vector to PHV
   phvwr	p.{rbuf_post_cmd_handle...rbuf_post_data_size},	\
		d.{cmd_handle...data_size}                                           

   // Set the table and program address 
   LOAD_TABLE_FBASE_IDX(R2N_RQ_CTX_TABLE_BASE, k.nvme_tgt_kivec0_paired_q_idx, 
                        Q_CTX_SIZE, Q_CTX_SIZE, r2n_rq_entry_push_start)
