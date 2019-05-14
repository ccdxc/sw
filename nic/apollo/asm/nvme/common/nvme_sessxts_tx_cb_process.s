#include "nvme_common.h"

//stage 0 program for SESSION XTS TX
//it simply checks pre/post xts rings and handsover the work to 
//respective thread

%%
    .param  nvme_sessprexts_tx_cb_process
    .param  nvme_sesspostxts_tx_cb_process
    .param  nvme_sessxts_tx_empty_ring_process

.align
nvme_sessxts_tx_cb_process:
    
    .brbegin
    brpri       r7[MAX_SESSXTS_TX_RINGS-1:0], [SESSPOSTXTS_TX_PRI, SESSPREXTS_TX_PRI]
    nop

    .brcase     SESSPREXTS_TX_RING_ID
        j       nvme_sessprexts_tx_cb_process
        nop     //BD Slot

    .brcase     SESSPOSTXTS_TX_RING_ID   
        j       nvme_sesspostxts_tx_cb_process
        nop     //BD Slot

    .brcase     MAX_SESSXTS_TX_RINGS
        j       nvme_sessxts_tx_empty_ring_process
        nop         //BD Slot
    .brend
