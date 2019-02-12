/*
 *    Implements the ooo qbase allocation stage of the RxDMA P4+ pipeline
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t2_tcp_rx_k.h"

struct phv_ p;
struct s4_t2_tcp_rx_k_ k;
struct s4_t2_tcp_rx_ooo_qbase_alloc_d d;

%%
    .align
    .param          tcp_ooo_qbase_cb_load 
tcp_rx_ooq_alloc_start:
    CAPRI_OPERAND_DEBUG(d.qbase)
    CAPRI_NEXT_TABLE_READ_OFFSET(2, TABLE_LOCK_EN,
                        tcp_ooo_qbase_cb_load,
                        k.common_phv_qstate_addr,
                        TCP_TCB_OOO_QADDR_OFFSET, TABLE_SIZE_512_BITS)
    phvwr.e         p.t2_s2s_ooo_qbase_addr, d.qbase
    nop
