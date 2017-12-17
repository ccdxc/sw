#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"
#include "../../../p4/nw/include/defines.h"

struct phv_ p;
struct rx_table_cpu_hash_k k;

%%
    .param cpu_rx_read_arqrx_pindex_start
    .param CAPRI_CPU_HASH_MASK
    .param CAPRI_CPU_MAX_ARQID
    .align

rx_table_cpu_hash:
    sne     c1, k.toeplitz_key2_data[3:0], P4PLUS_APPTYPE_CPU
    bcf     [c1], cpu_hash_calc_done
    nop

    CAPRI_OPERAND_DEBUG(r1.wx)
    phvwr   p.t0_s2s_hash, r1.wx

cpu_hash_calculate_id:
    /*
     * Calcualate ARQ QID/CPU ID from the hash. 
     * take the last bits from the hash based on the mask.
     * if the id received is greater than MAX CPU ID, use 0 as the qid
     */
    and     r2, r1.wx, CAPRI_CPU_HASH_MASK
    sle     c1, r2, CAPRI_CPU_MAX_ARQID
    add.!c1 r2, r0, r0
    phvwr   p.t0_s2s_arqrx_id, r2

    CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(cpu_rx_read_arqrx_pindex_start)
cpu_hash_calc_done:
    nop.e
    nop
