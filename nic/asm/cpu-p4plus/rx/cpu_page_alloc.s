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
    CAPRI_CLEAR_TABLE2_VALID

    phvwr       p.t0_s2s_page, d.u.page_alloc_d.page
    add         r3, r0, d.u.page_alloc_d.page

page_alloc_done:
    nop.e
    nop
