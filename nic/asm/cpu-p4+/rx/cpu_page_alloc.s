#include "ingress.h"
#include "INGRESS_p.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_rx_page_alloc_k k;
struct cpu_rx_page_alloc_d d;

%%
    .align

cpu_rx_page_alloc_start:
    phvwri      p.p4_intr_global_tm_oport, 8

    CAPRI_CLEAR_TABLE2_VALID

    phvwr       p.to_s3_page, d.u.page_alloc_d.page
    // Debug.TODO: Remove
    add         r3, r0, d.u.page_alloc_d.page

page_alloc_done:
    nop.e
    nop
