/*****************************************************************************
 *  rxdma_stage0.s: This assembly program provides the common entry point to
 *                   jump to various RXDMA P4+ programs.
 *                   1. Include the start labels used in your stage0 programs
 *                      in both the parameter and in the jump instrunction.
 *                   2. Use <this_program_name, stage0_label> when invoking
 *                      Capri MPU loader API capri_program_label_to_offset()
 *                      to derive the relative offset to program the queue
 *                      context structure.
 *                   3. Use <this_program_name> when invoking Capri MPU
 *                      loader API capri_program_to_base_addr() to derive
 *                      the base address for the stage 0 P4+ programs in the
 *                      table config engine.
 *****************************************************************************/

#include "INGRESS_p.h"

struct phv_ p;

%%
    .param      eth_rx_app_header
    .param      eth_rx_drop

//Keep offset 0 for none to avoid invoking unrelated program when
//qstate's pc_offset is not initialized
.align
rx_dummy:
    phvwr.e     p.capri_intr_drop, 1
    nop

//Do not change the order of this entry
//This has to align with the txdma_stage0.s program
.align
eth_rx_stage0:
    j eth_rx_app_header
    nop

//Do not change the order of this entry
//This has to align with the txdma_stage0.s program
.align
eth_tx_stage0_dummy:
    j eth_rx_drop
    nop
