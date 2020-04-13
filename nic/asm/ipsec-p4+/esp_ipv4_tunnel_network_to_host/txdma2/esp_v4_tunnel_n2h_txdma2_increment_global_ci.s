#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t1_k k;
struct tx_table_s1_t1_esp_v4_tunnel_n2h_txdma2_increment_global_ci_d d;
struct phv_ p;

%%
        .align
esp_v4_tunnel_n2h_txdma2_increment_global_ci:
    /* Check for the Barco Decrypt bug workaround slot on the ring */
    seq c1, d.ci[CAPRI_BARCO_GCM_DECRYPT_BUG_REQ_STRIDE_SHIFT-1:0], 0
    tblmincri.c1.f d.ci, ASIC_BARCO_RING_SLOTS_SHIFT, 2
    tblmincri.!c1.f d.ci, ASIC_BARCO_RING_SLOTS_SHIFT, 1
    nop.e
    nop
