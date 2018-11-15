#include "INGRESS_p.h"
#include "ingress.h"
#include "proxy-constants.h"
#include "capri-macros.h"

struct phv_ p;
struct gc_tx_read_descr_addr_read_descr_addr_d d;

%%
    .param          gc_tx_rnmdr_dummy
    .param          gc_tx_tnmdr_dummy

.align
gc_tx_read_rnmdr_addr:
    phvwr           p.{ring_entry1_descr_addr...ring_entry4_descr_addr}, \
                    d.{desc_addr1...desc_addr4}
    phvwr           p.{ring_entry5_descr_addr...ring_entry8_descr_addr}, \
                    d.{desc_addr5...desc_addr8}
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, gc_tx_rnmdr_dummy)
    nop.e
    nop

.align
gc_tx_read_tnmdr_addr:
    phvwr           p.{ring_entry1_descr_addr...ring_entry4_descr_addr}, \
                    d.{desc_addr1...desc_addr4}
    phvwr           p.{ring_entry5_descr_addr...ring_entry8_descr_addr}, \
                    d.{desc_addr5...desc_addr8}
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, gc_tx_tnmdr_dummy)
    nop.e
    nop

gc_tx_read_descr_fatal_error:
    phvwri p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    illegal
    nop.e
    nop
