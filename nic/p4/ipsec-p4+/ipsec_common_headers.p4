#include "ipsec_defines.h"

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

/* Definition of BARCO Shadow PI/CI/Stats block in HBM used by all BARCO rings */
header_type barco_shadow_params_d_t {
    fields {
       pi         : 16;
       ci         : 16;
       stat_qfull : 32;
       stat_q_hwm : 32;
       stat_rsvd  : 32;
    }
}


#define BARCO_SHADOW_PARAMS    pi, ci, stat_qfull, stat_q_hwm, stat_rsvd
#define GENERATE_BARCO_SHADOW_PARAMS_D                                       \
    modify_field(barco_shadow_params_d.pi, pi);                              \
    modify_field(barco_shadow_params_d.ci, ci);                              \
    modify_field(barco_shadow_params_d.stat_qfull, stat_qfull);              \
    modify_field(barco_shadow_params_d.stat_qfull, stat_q_hwm);              \
    modify_field(barco_shadow_params_d.stat_rsvd, stat_rsvd);

