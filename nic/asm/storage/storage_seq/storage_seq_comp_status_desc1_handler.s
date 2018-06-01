/*****************************************************************************
 *  seq_comp_status_desc1_handler:
 *****************************************************************************/

#include "storage_asm_defines.h"
#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl1_k k;
struct s1_tbl1_seq_comp_status_desc1_handler_d d;
struct phv_ p;

/*
 * Registers usage:
 */
#define r_pad_buf_addr              r1  // pointer to pad buffer

%%

storage_seq_comp_status_desc1_handler_start:

    CLEAR_TABLE1
    phvwrpair	p.seq_kivec5_data_len, d.data_len, \
                p.{seq_kivec5_stop_chain_on_error...seq_kivec5_copy_src_dst_on_error}, \
    	        d.{stop_chain_on_error...copy_src_dst_on_error}
    add         r_pad_buf_addr, d.pad_buf_addr, r0
    phvwr       p.seq_kivec5_pad_buf_addr, r_pad_buf_addr
    phvwr       p.pad_buf_addr_addr, r_pad_buf_addr.dx
    
    phvwrpair	p.seq_kivec3_comp_buf_addr, d.comp_buf_addr, \
                p.seq_kivec3_pad_boundary_shift, d.pad_boundary_shift
    phvwr       p.seq_kivec4_pad_boundary_shift, d.pad_boundary_shift

    // aol_dst_vec_addr points to SGL PDMA destination when SGL PDMA (full) transfer
    // is applicable, unless AOL padding is enabled (see below)
    phvwrpair.e	p.seq_kivec2_sgl_pdma_dst_addr, d.aol_dst_vec_addr, \
                p.seq_kivec2_sgl_vec_addr, d.sgl_vec_addr

    phvwrpair	p.seq_kivec6_aol_src_vec_addr, d.aol_src_vec_addr, \
                p.seq_kivec6_aol_dst_vec_addr, d.aol_dst_vec_addr

