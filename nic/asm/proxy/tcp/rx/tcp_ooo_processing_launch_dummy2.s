#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s5_t1_tcp_rx_k.h"

struct phv_ p;
struct s5_t1_tcp_rx_k_ k;
struct s5_t1_tcp_rx_d d;

%%
    .align
    .param tcp_ooo_qbase_cb_load 
tcp_ooo_processing_launch_dummy2:
    CAPRI_NEXT_TABLE_READ_OFFSET(3, TABLE_LOCK_EN,
                        tcp_ooo_qbase_cb_load,
                        k.common_phv_qstate_addr,
                        TCP_TCB_OOO_QADDR_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop


