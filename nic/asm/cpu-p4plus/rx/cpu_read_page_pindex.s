#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_rx_read_cpu_page_k k;
struct cpu_rx_read_cpu_page_d d;

%%
    .param cpu_rx_page_alloc_start
    .param RNMPR_TABLE_BASE
    .align

cpu_rx_read_page_pindex_start:
    CAPRI_CLEAR_TABLE2_VALID
    
    add     r4, r0, d.{u.read_cpu_page_d.page_pindex}.wx
    andi    r4, r4, ((1 << CAPRI_RNMPR_RING_SHIFT) - 1)
    phvwr   p.s2_t2_s2s_page_pindex, r4

table_read_page_alloc:
    addui   r3, r0, hiword(RNMPR_TABLE_BASE)
    addi    r3, r0, loword(RNMPR_TABLE_BASE)
 	CAPRI_NEXT_TABLE_READ_INDEX(2, 
                                r4, 
                                TABLE_LOCK_DIS,
                                cpu_rx_page_alloc_start,
	                            r3, 
                                RNMPR_TABLE_ENTRY_SIZE_SHFT,
                                TABLE_SIZE_64_BITS)
	nop.e
	nop   
