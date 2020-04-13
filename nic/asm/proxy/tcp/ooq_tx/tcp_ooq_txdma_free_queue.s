#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t1_ooq_tcp_tx_k.h"

struct phv_ p;
struct s1_t1_ooq_tcp_tx_k_ k;
struct s1_t1_ooq_tcp_tx_free_ooq_d d;

%%
    .align
    .param          TCP_OOQ_TABLE_BASE
tcp_ooq_free_queue:
    and             r1, d.ooq_free_pi, ASIC_TCP_ALLOC_OOQ_RING_MASK
    tbladd.f        d.ooq_free_pi, 1
    CAPRI_CLEAR_TABLE_VALID(1)

    /*
     * Write qbase address to OOQ_TABLE[pi]
     */
    addui           r3, r0, hiword(TCP_OOQ_TABLE_BASE)
    addi            r3, r3, loword(TCP_OOQ_TABLE_BASE)
    add             r3, r3, r1, TCP_OOQ_TABLE_ENTRY_SIZE_SHFT
    phvwr           p.ooq_slot_descr_addr, k.to_s1_qbase_addr
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ooq_ring_entry_dma_cmd, r3, ooq_slot_descr_addr, ooq_slot_descr_addr)

    /*
     * Write semaphore CI value
     */
    add             r2, d.ooq_free_pi, ASIC_TCP_ALLOC_OOQ_RING_SIZE
    phvwr           p.ooq_free_pi_index, r2.wx
    addi            r3, r0, ASIC_SEM_TCP_OOQ_ALLOC_CI_RAW_ADDR
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ooq_alloc_ci_dma_cmd, r3, ooq_free_pi_index, ooq_free_pi_index)

    nop.e
    nop
