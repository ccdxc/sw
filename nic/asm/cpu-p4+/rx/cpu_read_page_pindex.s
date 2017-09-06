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
    
    phvwr   p.s2_t2_s2s_page_pindex, d.u.read_cpu_page_d.page_pindex

table_read_page_alloc:
    addi    r3, r0, RNMPR_TABLE_BASE
    //addi    r3, r0, 0xa55b2000 
 	CAPRI_NEXT_TABLE2_READ(d.u.read_cpu_page_d.page_pindex, 
                           TABLE_LOCK_EN,
                           cpu_rx_page_alloc_start,
	                       r3, 
                           RNMPR_TABLE_ENTRY_SIZE_SHFT,
	                       0,
                           TABLE_SIZE_512_BITS)
	nop.e
	nop   
