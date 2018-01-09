#include "app_redir_common.h"

struct phv_                     p;
struct proxyr_tx_start_k        k;
struct proxyr_tx_start_d        d;

/*
 * Registers usage
 */
#define r_ci                        r1  // my_txq onsumer index
#define r_pi                        r2  // my_txq producer index
#define r_my_txq_qid                r3
#define r_qstate_addr               r4
#define r_return                    r5
#define r_scratch                   r6

%%
    .param          proxyr_s1_my_txq_entry_consume
    .param          proxyr_s1_flow_key_post_read
    .param          proxyr_err_stats_inc
    
    .align

proxyr_s0_tx_start:
        
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
     
    CAPRI_CLEAR_TABLE0_VALID
    phvwr       p.common_phv_qstate_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR

    /*
     * qid is our flow ID context
     */
    phvwr       p.to_s1_my_txq_lif, CAPRI_INTRINSIC_LIF     // delay slot
    phvwr       p.to_s1_my_txq_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE
    phvwr       p.to_s1_my_txq_qid, CAPRI_TXDMA_INTRINSIC_QID

    phvwr       p.common_phv_proxyrcb_flags, d.{u.start_d.proxyrcb_flags}.hx
    phvwr       p.common_phv_redir_span_instance, d.u.start_d.redir_span
    
    /*
     * Also prefill certain meta header fields;
     * FTE expects p4plus_cpu_pkt_lif/qtype/qid in host order,
     * whereas L7 header fields should all be in network order
     */
    phvwr       p.p4plus_cpu_pkt_lif, CAPRI_INTRINSIC_LIF.hx
    phvwr       p.p4plus_cpu_pkt_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE
    add         r_my_txq_qid, r0, CAPRI_TXDMA_INTRINSIC_QID
    phvwr       p.p4plus_cpu_pkt_qid, r_my_txq_qid.wx
    phvwr       p.pen_proxyr_hdr_v1_flow_id, r_my_txq_qid

    phvwr       p.common_phv_chain_ring_base, d.{u.start_d.chain_rxq_base}.dx
    phvwr       p.common_phv_chain_ring_size_shift, d.u.start_d.chain_rxq_ring_size_shift
    phvwr       p.common_phv_chain_entry_size_shift, d.u.start_d.chain_rxq_entry_size_shift
    phvwr       p.common_phv_chain_ring_index_select, d.u.start_d.chain_rxq_ring_index_select
    phvwr       p.to_s5_chain_ring_indices_addr, d.{u.start_d.chain_rxq_ring_indices_addr}.dx
    
    /*
     * Two sentinels surround the programming of CB byte sequence:
     * proxyrcb_deactivate must be false and proxyrcb_activate must
     * be true to indicate readiness.
     *
     * Note that proxyrcb_activate is part of proxyrcb_extra_t and
     * its evaluation will be deferred until proxyrcb_extra_t is read
     * in stage 1.
     */
    phvwr       p.to_s1_proxyrcb_deactivate, d.u.start_d.proxyrcb_deactivate
    add         r_qstate_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR, \
                PROXYRCB_TABLE_FLOW_KEY_OFFSET                  
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                          proxyr_s1_flow_key_post_read,
                          r_qstate_addr,
                          TABLE_SIZE_512_BITS)
    /*
     * PI assumed to have been incremented by doorbell write by a producer program;
     * double check for queue not empty in case we somehow got erroneously scheduled.
     */
    phvwr       p.to_s1_my_txq_ring_size_shift, d.u.start_d.my_txq_ring_size_shift
    add         r_ci, r0, d.{u.start_d.ci_0}.hx
    add         r_pi, r0, d.{u.start_d.pi_0}.hx
    mincr       r_ci, d.u.start_d.my_txq_ring_size_shift, r0
    mincr       r_pi, d.u.start_d.my_txq_ring_size_shift, r0
    beq         r_ci, r_pi, _my_txq_ring_empty
    phvwr       p.to_s1_my_txq_ci_curr, r_ci    // delay slot

    /*
     * Launch read of descriptor at current CI.
     * Note that my_txq_base and corresponding CI/PI, once programmed,
     * are never cleared (because doing so can cause Tx scheduler lockup).
     * What can get cleared is the proxyrcb_active flag which would tell this
     * program to properly consume and free the descriptor along with 
     * any pages embedded within.
     */    
    add         r_scratch, r0, d.u.start_d.my_txq_entry_size_shift
    sllv        r_ci, r_ci, r_scratch
    add         r_ci, r_ci, d.{u.start_d.my_txq_base}.dx
    CAPRI_NEXT_TABLE_READ(0,
                          TABLE_LOCK_DIS,
                          proxyr_s1_my_txq_entry_consume,
                          r_ci,
                          TABLE_SIZE_64_BITS)
    nop.e
    nop    
    
     
/*
 * Early exit: my TxQ ring actually empty when entered
 */
_my_txq_ring_empty:

    PROXYRCB_ERR_STAT_INC_LAUNCH(3, r_qstate_addr,
                                 CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR,
                                 p.t3_s2s_inc_stat_txq_empty)
    nop.e
    nop

    
