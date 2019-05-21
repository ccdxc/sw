#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t0_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s4_t0_k_ k;
struct s4_t0_nvme_sessprexts_tx_ip_desc_process_d d;

%%

.align
nvme_sessprexts_tx_ip_desc_process:
    // since phv is always initialized to 0 to start with,
    // all the address, offset, length fields would have initial values of 0.

    //XXX TBD: if needed prp pointers endianness need to be changed
    phvwr       p.ip_desc_A0, d.prp1
   
    bbeq        k.to_s4_info_prp2_valid, 0, skip_prp2
    phvwrpair   p.ip_desc_O0, k.to_s4_info_prp1_offset, \
                p.ip_desc_L0, k.to_s4_info_prp1_bytes   //BD Slot

    //XXX TBD: if needed prp pointers endianness need to be changed
    phvwr       p.ip_desc_A1, d.prp2
    phvwrpair   p.ip_desc_O1, r0, \
                p.ip_desc_L1, k.to_s4_info_prp2_bytes

skip_prp2: 

    nop.e
    CAPRI_SET_TABLE_0_VALID(0)  //Exit Slot
