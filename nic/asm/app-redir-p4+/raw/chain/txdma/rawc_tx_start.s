#include "app_redir_common.h"

struct phv_                     p;
struct rawc_tx_start_k          k;
struct rawc_tx_start_start_d    d;

%%
    .param          rawc_s1_my_txq_entry_consume
    
    .align

rawc_s0_tx_start:
        
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
    seq         c1, d.my_txq_base, r0
    bcf         [c1], my_txq_not_cfg
     
    /*
     * qid is our flow ID context
     */
    phvwr       p.to_s1_my_txq_lif, CAPRI_INTRINSIC_LIF     // delay slot
    phvwr       p.to_s1_my_txq_qtype, CAPRI_TXDMA_INTRINSIC_QTYPE
    phvwr       p.to_s1_my_txq_qid, CAPRI_TXDMA_INTRINSIC_QID

    phvwr       p.common_phv_chain_txq_base, d.chain_txq_base
    phvwr       p.common_phv_chain_txq_ring_size_shift, d.chain_txq_ring_size_shift
    phvwr       p.common_phv_chain_txq_entry_size_shift, d.chain_txq_entry_size_shift
    phvwr       p.common_phv_chain_txq_lif, d.chain_txq_lif
    phvwr       p.common_phv_chain_txq_qtype, d.chain_txq_qtype
    phvwr       p.common_phv_chain_txq_qid, d.chain_txq_qid
    phvwr       p.common_phv_chain_txq_ring, d.chain_txq_ring
    
    phvwr       p.to_s1_chain_txq_ring_indices_addr, d.chain_txq_ring_indices_addr
    phvwr       p.to_s1_my_txq_ring_size_shift, d.my_txq_ring_size_shift
    
    /*
     * PI assumed to have been incremented by doorbell write by a producer program;
     * double check for queue not empty in case we somehow got erroneously scheduled.
     */
    add         r3, r0, d.{ci_0}.hx
    mincr       r3, d.my_txq_ring_size_shift, r0
    add         r4, r0, d.{pi_0}.hx
    mincr       r4, d.my_txq_ring_size_shift, r0
    beq         r3, r4, my_txq_ring_empty
    phvwr       p.to_s1_my_txq_ci_curr, r3        // delay slot

    /*
     * Launch read of descriptor at current CI
     */    
    add         r4, r0, d.my_txq_entry_size_shift
    sllv        r3, r3, r4
    add         r3, r3, d.{my_txq_base}.wx
    CAPRI_NEXT_TABLE_READ(0,
                          TABLE_LOCK_DIS,
                          rawc_s1_my_txq_entry_consume,
                          r3,
                          TABLE_SIZE_64_BITS)
    nop.e
    nop    
    
my_txq_not_cfg:

    /*
     * Discard packet due to error in qstate configuration
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
