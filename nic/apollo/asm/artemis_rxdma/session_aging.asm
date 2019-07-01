
#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct session_aging_k  k;
struct session_aging_d  d;
struct phv_             p;

// 5 min timeout. 833 cycles per usec
#define TIMEOUT_IN_CYCLES (5*60*1000000*833)
#define TIMEOUT_IN_CYCLES_L 0x2F336300
#define TIMEOUT_IN_CYCLES_U 0x3A
    
%%
    .param session2flow
    
session_aging:

    // If session entry is not valid just skip the entry and exit the program.
    bbne        d.session_aging_d.entry_valid, TRUE, exit

    // If the session timeout did not exceed the timeout, skip the entry and
    // exit the program
    sub         r1, r4, d.session_aging_d.timestamp  //BD slot
    addui       r2, r0, TIMEOUT_IN_CYCLES_U
    addi        r2, r2, TIMEOUT_IN_CYCLES_L
    blt         r1, r2, exit
    nop
    
    // If entry is valid and timeout exceeded, Construct packet to send to
    // P4+ assist
    phvwr.e     p.rx_to_tx_hdr4_session_ind, k.capri_rxdma_intr_qid
    nop
    
exit:
    phvwr.e     p.capri_intr_drop, 1
    nop
