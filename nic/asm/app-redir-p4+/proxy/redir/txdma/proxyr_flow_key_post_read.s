#include "app_redir_common.h"

struct phv_                                 p;
struct proxyr_flow_key_k                    k;
struct proxyr_flow_key_flow_key_post_read_d d;

/*
 * Registers usage
 */
#define r_qstate_addr               r3

%%
    .param      proxyr_err_stats_inc
    
    .align

/*
 * Flow key 6-tuple and other flags have been read from qstate.
 * These are now used to populate the proxy V1 header.
 */    
proxyr_s1_flow_key_post_read:

    CAPRI_CLEAR_TABLE1_VALID

    /*
     * Two sentinels surround the programming of CB byte sequence:
     * proxyrcb_deactivated must be false and proxyrcb_activated must
     * be true to indicate readiness.
     */
    sne         c1, k.to_s1_proxyrcb_deactivate, PROXYRCB_DEACTIVATE
    seq.c1      c1, d.proxyrcb_activate, PROXYRCB_ACTIVATE
    b.!c1       _proxyrcb_not_ready
    
    /*
     * Populate more meta header fields with flow key.
     * Note: fields are written individually here due to NCC (though not
     * all NCC versions apparently) reordering of fields in k-vec.
     */
    phvwr       p.p4plus_cpu_pkt_lkp_vrf, d.{vrf}.hx
    phvwr       p.pen_proxyr_hdr_v1_vrf, d.vrf
    phvwr       p.{pen_proxyr_hdr_v1_ip_sa, pen_proxyr_hdr_v1_ip_da}, \
                d.{ip_sa, ip_da}
    phvwr       p.{pen_proxyr_hdr_v1_sport, pen_proxyr_hdr_v1_dport}, \
                d.{sport, dport}
    phvwr       p.pen_proxyr_hdr_v1_af, d.af
    phvwr.e     p.pen_proxyr_hdr_v1_ip_proto, d.ip_proto
    nop

    .align
    
/*
 * CB has been de-activated or never made ready
 */
_proxyrcb_not_ready:
 
    PROXYRCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                                 k.{common_phv_qstate_addr_sbit0_ebit5... \
                                    common_phv_qstate_addr_sbit30_ebit33},
                                 PROXYRCB_STAT_CB_NOT_READY_BYTE_OFFS)
    phvwri.e    p.common_phv_do_cleanup_discard, TRUE
    nop
