/******************************************************************
 * cpu_rxtx_common.p4
 * This file contains headers common to CPU TXDMA and RXDMA programs
 *******************************************************************/

header_type quiesce_pkt_trlr_t {
    fields {
        timestamp               : 32; 
    }    
}


