#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;

%%

// RXDMA program stage 0 needs table 0 valid bit to be set so 
// that qstate loaded in stage 0 can be offset by 64bytes. So
// invoke dummy_program to keep table 0 valid bit in phv, which 
// eventually after stage7 will result in invoking stage0 program
// with qstate offset by 64bytes.

.align
req_rx_recirc_mpu_only_process:
    nop.e
    nop
