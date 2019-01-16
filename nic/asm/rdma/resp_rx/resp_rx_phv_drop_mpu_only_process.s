#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s7_t0_k k;

%%

// assumption is that this program gets invoked thru table 0,
// only when resp_rx stage 0 has already set p4_intr_global_drop
// bit to 1. this program keeps bubbling down as mpu_only program 
// all the way till stage_7
// this is needed because, if all table valid bits are set to 0
// eth loads a program in stage 1, which can lead to loading of
// an invalid address, causing a PCAT error eventually

.align
resp_rx_phv_drop_mpu_only_process:
    // do nothing. let table valid bit be 1, 
    // so that this program keeps loading itself
    nop.e
    nop // Exit Slot


