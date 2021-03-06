/*****************************************************************************
 *  seq_xts_status_desc1_handler:
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl1_k k;
struct s1_tbl1_seq_xts_status_desc1_handler_d d;
struct phv_ p;

/*
 * Registers usage:
 */
#define r_src_qaddr                 r7  // for SEQ_METRICS_TABLE_COMMIT

%%
    SEQ_METRICS_PARAMS()

storage_seq_xts_status_desc1_handler:

    // Relaunch metrics commit for table 1
    SEQ_METRICS0_TABLE1_COMMIT(SEQ_KIVEC5XTS_SRC_QADDR)
    
    phvwrpair   p.{seq_kivec5xts_data_len...seq_kivec5xts_blk_boundary_shift}, \
                d.{data_len...blk_boundary_shift}, \
                p.{seq_kivec5xts_stop_chain_on_error...seq_kivec5xts_desc_vec_push_en}, \
                d.{stop_chain_on_error...desc_vec_push_en}
    phvwrpair.e	p.seq_kivec2xts_sgl_pdma_dst_addr, d.sgl_pdma_dst_addr, \
                p.seq_kivec2xts_decr_buf_addr, d.decr_buf_addr
    phvwr       p.seq_kivec7xts_comp_sgl_src_addr, d.comp_sgl_src_addr


