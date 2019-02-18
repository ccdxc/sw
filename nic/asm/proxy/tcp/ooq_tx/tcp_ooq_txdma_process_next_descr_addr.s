#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_ooq_tcp_tx_k.h"

struct phv_ p;
struct s1_t0_ooq_tcp_tx_k_ k;
struct s1_t0_ooq_tcp_tx_process_next_descr_addr_d d;

%%
    .align
    .param          tcp_ooq_txdma_read_descr
tcp_ooq_txdma_process_next_descr_addr:
    CAPRI_OPERAND_DEBUG(d.descr_addr)
    add             r3, d.descr_addr, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tcp_ooq_txdma_read_descr, 
                        r3, TABLE_SIZE_512_BITS)
    nop.e
    nop
