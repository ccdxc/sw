#include "nvme_common.h"

//stage 0 program for SESSION DGST TX
//it simply checks pre/post dgst rings and handsover the work to 
//respective thread

%%
    .param  nvme_sesspredgst_tx_cb_process
    .param  nvme_sesspostdgst_tx_cb_process
    .param  nvme_sessdgst_tx_empty_ring_process

.align
nvme_sessdgst_tx_cb_process:
    
    .brbegin
    brpri       r7[MAX_SESSDGST_TX_RINGS-1:0], [SESSPOSTDGST_TX_PRI, SESSPREDGST_TX_PRI]
    nop

    .brcase     SESSPREDGST_TX_RING_ID
        j       nvme_sesspredgst_tx_cb_process
        nop     //BD Slot

    .brcase     SESSPOSTDGST_TX_RING_ID   
        j       nvme_sesspostdgst_tx_cb_process
        nop     //BD Slot

    .brcase     MAX_SESSDGST_TX_RINGS
        j       nvme_sessdgst_tx_empty_ring_process
        nop     //Exit Slot
    .brend
