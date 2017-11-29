#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"
#include "../../../p4/nw/include/defines.h"

struct phv_ p;
struct rx_table_cpu_hash_k k;

%%
    .param cpu_rx_read_arqrx_pindex_start
    .align

cpu_rx_hash_start:
    sne     c1, k.toeplitz_key2_data[3:0], P4PLUS_APPTYPE_CPU
    bcf     [c1], cpu_hash_calc_done
    nop

    CAPRI_OPERAND_DEBUG(r1.wx)
    phvwr   p.t0_s2s_hash, r1.wx

    CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(cpu_rx_read_arqrx_pindex_start)
cpu_hash_calc_done:
    nop.e
    nop
