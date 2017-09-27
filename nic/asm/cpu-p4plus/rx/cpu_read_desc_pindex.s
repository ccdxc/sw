#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_rx_read_cpu_desc_k k;
struct cpu_rx_read_cpu_desc_d d;

%%
    .param cpu_rx_desc_alloc_start
    .param RNMDR_TABLE_BASE
    .align

cpu_rx_read_desc_pindex_start:
    CAPRI_CLEAR_TABLE1_VALID
    add     r4, r0, d.{u.read_cpu_desc_d.desc_pindex}.wx
    andi    r4, r4, ((1 << CAPRI_RNMDR_RING_SHIFT) - 1)
    phvwr   p.s2_t1_s2s_desc_pindex, r4 

table_read_desc_alloc:
    addui   r3, r0, hiword(RNMDR_TABLE_BASE)
    addi    r3, r0, loword(RNMDR_TABLE_BASE)
 	CAPRI_NEXT_TABLE_READ_INDEX(1, 
                                r4, 
                                TABLE_LOCK_DIS,
                                cpu_rx_desc_alloc_start,
	                            r3, 
                                RNMDR_TABLE_ENTRY_SIZE_SHFT,
                                TABLE_SIZE_64_BITS)
	nop.e
	nop   
