/*****************************************************************************
 *  seq_comp_status_handler: Store the compression status in K+I vector. Load
 *                           SGL address for next stage to do the PDMA.
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s2_tbl_k k;
struct s2_tbl_seq_comp_status_handler_d d;
struct phv_ p;

%%
   .param storage_tx_seq_comp_sgl_handler_start

storage_tx_seq_comp_status_handler_start:

   // Store the data length and error status in the K+I vector for later use
   phvwr	p.storage_kivec4_data_len, d.data_len
   phvwr	p.storage_kivec5_status_err, d.err
  
   // Set the table and program address 
   LOAD_TABLE_FOR_ADDR_PARAM(STORAGE_KIVEC4_SGL_ADDR, 
                             STORAGE_DEFAULT_TBL_LOAD_SIZE,
                             storage_tx_seq_comp_sgl_handler_start)

