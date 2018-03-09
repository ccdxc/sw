#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"
#include "../../../p4/iris/include/defines.h"

struct phv_ p;
struct rx_table_cpu_hash_k k;

%%
    .param cpu_rx_read_arqrx_pindex_start
    .param CAPRI_CPU_HASH_MASK
    .param CAPRI_CPU_MAX_ARQID
    .align

cpu_hash_calculate_id:
    CAPRI_OPERAND_DEBUG(r1.wx)

    /*
     * Entered from asm/common-p4+/rx_table_cpu_hash.s after a HW hash calculation,
     * Calcualate ARQ QID/CPU ID from the hash. 
     * take the last bits from the hash based on the mask.
     * if the id received is greater than MAX CPU ID, use 0 as the qid
     */
    TOEPLITZ_HASH_CALC_ID(r1,
                          CAPRI_CPU_HASH_MASK,
                          CAPRI_CPU_MAX_ARQID,
                          c1)
    phvwr   p.t0_s2s_arqrx_id, r1

    CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(cpu_rx_read_arqrx_pindex_start)
cpu_hash_calc_done:
    nop.e
    nop
