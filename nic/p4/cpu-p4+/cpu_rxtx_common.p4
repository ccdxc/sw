/******************************************************************
 * cpu_rxtx_common.p4
 * This file contains headers common to CPU TXDMA and RXDMA programs
 *******************************************************************/
#define CPU_HBM_ADDRESS_WIDTH   40

header_type quiesce_pkt_trlr_t {
    fields {
        timestamp               : 32; 
    }    
}

/*
 * flags bits
 * 0 : do not update dot1q
 * 1 : Add/update  VLAN tag
 */
header_type cpu_to_p4plus_header_t {
    fields {
        flags                   : 16;
        src_lif                 : 16;

        // hw_vlan_id
        hw_vlan_id              : 16;

        // offsets
        l2_offset               : 16;
        
        pad                     : 3;
        tm_oq                   : 5;
    }    
}

