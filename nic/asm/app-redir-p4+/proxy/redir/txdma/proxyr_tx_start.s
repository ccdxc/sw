#include "app_redir_common.h"

struct phv_                     p;
struct proxyr_tx_start_k        k;
struct proxyr_tx_start_d        d;

%%
    .param          proxyr_s1_my_txq_entry_consume
    
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
    
    /*
     * do nothing if my_txq not configured
     */
    seq         c1, d.u.start_d.my_txq_base, r0
    bcf         [c1], my_txq_cfg_err
     
    /*
     * qid is our flow ID context
     */
    phvwr       p.to_s1_my_txq_lif, CAPRI_INTRINSIC_LIF     // delay slot
    phvwr       p.to_s1_my_txq_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE
    phvwr       p.to_s1_my_txq_qid, CAPRI_TXDMA_INTRINSIC_QID
    phvwr       p.to_s4_qstate_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR

    /*
     * Also prefill certain meta header fields;
     * FTE expects p4plus_cpu_pkt_lif/qtype/qid in host order,
     * whereas L7 header fields should all be in network order
     */
    phvwr       p.p4plus_cpu_pkt_lif, CAPRI_INTRINSIC_LIF.hx
    phvwr       p.p4plus_cpu_pkt_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE
    add         r3, r0, CAPRI_TXDMA_INTRINSIC_QID
    phvwr       p.p4plus_cpu_pkt_qid, r3.wx
    phvwr       p.pen_proxyr_hdr_v1_flow_id, r3

    phvwr.c1    p.common_phv_chain_ring_base, d.u.start_d.chain_rxq_base
    phvwr.c1    p.common_phv_chain_ring_size_shift, d.u.start_d.chain_rxq_ring_size_shift
    phvwr.c1    p.common_phv_chain_entry_size_shift, d.u.start_d.chain_rxq_entry_size_shift
    phvwr.c1    p.common_phv_chain_ring_index_select, d.u.start_d.chain_rxq_ring_index_select
    phvwr.c1    p.to_s5_chain_ring_indices_addr, d.u.start_d.chain_rxq_ring_indices_addr
    
    phvwr       p.common_phv_dol_flags, d.u.start_d.dol_flags
    
    /*
     * PI assumed to have been incremented by doorbell write by a producer program;
     * double check for queue not empty in case we somehow got erroneously scheduled.
     */
    add         r3, r0, d.{u.start_d.ci_0}.hx
    mincr       r3, d.u.start_d.my_txq_ring_size_shift, r0
    add         r4, r0, d.{u.start_d.pi_0}.hx
    mincr       r4, d.u.start_d.my_txq_ring_size_shift, r0
    beq         r3, r4, my_txq_ring_empty
    phvwr       p.to_s1_my_txq_ci_curr, r3        // delay slot

    /*
     * Launch read of descriptor at current CI
     */    
    add         r4, r0, d.u.start_d.my_txq_entry_size_shift
    sllv        r3, r3, r4
    add         r3, r3, d.{u.start_d.my_txq_base}.wx
    CAPRI_NEXT_TABLE_READ(0,
                          TABLE_LOCK_DIS,
                          proxyr_s1_my_txq_entry_consume,
                          r3,
                          TABLE_SIZE_64_BITS)
    nop.e
    nop    
    
my_txq_cfg_err:

    /*
     * My TxQ not configured
     * TODO: add stats here
     */
    nop.e
    nop

my_txq_ring_empty:

    /*
     * Early exit: my TxQ ring actually empty when entered
     * TODO: add stats here
     */
    nop.e
    nop

    
