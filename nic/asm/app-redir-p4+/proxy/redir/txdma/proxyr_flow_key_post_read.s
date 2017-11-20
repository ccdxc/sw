#include "app_redir_common.h"

struct phv_                                 p;
struct proxyr_flow_key_k                    k;
struct proxyr_flow_key_flow_key_post_read_d d;

%%

    .param      proxyr_s6_chain_xfer
    .param      proxyr_s6_cleanup_discard
    .align
    
proxyr_s5_flow_key_post_read:

    CAPRI_CLEAR_TABLE0_VALID

    /*
     * Proceed only if mpage successfully allocated
     */
    sne         c1, k.common_phv_mpage_sem_pindex_full, r0
    bcf         [c1], cleanup_discard_launch
    add         r3, r0, k.{to_s5_chain_ring_indices_addr}.wx    // delay slot
    beq         r3, r0, cleanup_discard_launch
    nop

    /*
     * Populate more meta header fields with flow key
     */
    phvwr       p.pen_proxyr_hdr_v1_vrf, d.vrf
    phvwr       p.pen_proxyr_hdr_v1_ip_sa, d.ip_sa
    phvwr       p.pen_proxyr_hdr_v1_ip_da, d.ip_da
    phvwr       p.pen_proxyr_hdr_v1_sport, d.sport
    phvwr       p.pen_proxyr_hdr_v1_dport, d.dport
    phvwr       p.pen_proxyr_hdr_v1_af, d.af
    phvwr       p.pen_proxyr_hdr_v1_ip_proto, d.ip_proto
    //phvwr       p.pen_proxyr_hdr_v1_tcp_flags, ???
     
    /*
     * Chain to ARM RxQ: access HBM queue index table directly
     */
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN,
                          proxyr_s6_chain_xfer,
                          r3,
                          TABLE_SIZE_512_BITS)
    nop.e
    nop

cleanup_discard_launch:    

    /*
     * Launch common cleanup code for next stage
     * TODO: add stats here
     */
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, proxyr_s6_cleanup_discard)
    nop.e
    nop
