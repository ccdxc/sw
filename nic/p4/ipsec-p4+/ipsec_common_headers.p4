#include "ipsec_defines.h"

header_type p4plus_to_p4_ipsec_header_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        flags               : 8;
        udp_opt_bytes       : 8;
        rsvd                : 24;
        ip_id_delta         : 16;
        tcp_seq_delta       : 32;
        gso_start           : 14;
        gso_offset          : 14;
        byte_align_pad      : 3;
        gso_valid           : 1;
        vlan_tag            : 16;
        ipsec_pad1          : 120;
    }
}


header_type barco_descriptor_t {
    fields {
        A0_addr : ADDRESS_WIDTH;
        O0      : AOL_OFFSET_WIDTH;
        L0      : AOL_LENGTH_WIDTH;
        A1_addr : ADDRESS_WIDTH;
        O1      : AOL_OFFSET_WIDTH;
        L1      : AOL_LENGTH_WIDTH;
    }
}

header_type barco_zero_content_t {
    fields {
        A2_addr : ADDRESS_WIDTH;
        O2      : AOL_OFFSET_WIDTH;
        L2      : AOL_LENGTH_WIDTH;
        nextAddress : ADDRESS_WIDTH;
        rsvd    : ADDRESS_WIDTH;
    }
}

header_type barco_request_t {
    fields {
        input_list_address                  : 64;
        output_list_address                 : 64;
        command                             : 32;
        key_desc_index                      : 32;
        iv_address                          : 64;
        auth_tag_addr                       : 64;
        header_size                         : 32;
        status_address                      : 64;
        opaque_tag_value                    : 32;
        opaque_tag_write_en                 : 1;
        rsvd1                               : 31;
        sector_size                         : 16;
        application_tag                     : 16;
        sector_num                          : 32;
        doorbell_address                    : 64;
        doorbell_data                       : 64;
    }
}

