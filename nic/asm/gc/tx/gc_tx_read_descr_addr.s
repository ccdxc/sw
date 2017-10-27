#include "INGRESS_p.h"
#include "ingress.h"
#include "proxy-constants.h"
#include "capri-macros.h"

struct phv_ p;
struct gc_tx_read_descr_addr_read_descr_addr_d d;

%%
    .param          gc_tx_read_rnmdr_descr
    .param          gc_tx_read_tnmdr_descr

.align
gc_tx_read_rnmdr_addr:
    phvwr           p.common_phv_desc_addr, d.desc_addr
    add             r3, d.desc_addr, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, gc_tx_read_rnmdr_descr, d.desc_addr, TABLE_SIZE_512_BITS)
    nop.e
    nop

.align
gc_tx_read_tnmdr_addr:
    phvwr           p.common_phv_desc_addr, d.desc_addr
    add             r3, d.desc_addr, NIC_DESC_ENTRY_0_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, gc_tx_read_tnmdr_descr, d.desc_addr, TABLE_SIZE_512_BITS)
    nop.e
    nop
