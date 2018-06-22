/***************************************************
 * cpu_rx_common.p4
 **************************************************/
header_type arq_pi_d_t {
    fields {
        arq_pindex              : 32;
    }    
}

#define ARQ_PI_PARAMS       \
arq_pindex

#define GENERATE_ARQ_PI_D(_d_struct)  \
    modify_field(_d_struct.arq_pindex, arq_pindex);


header_type p4_to_p4plus_cpu_header_ext_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        ip_proto            : 8;
        l4_sport            : 16;
        l4_dport            : 16;
        packet_type         : 2;
        packet_len          : 14;
        ip_sa               : 128;
        ip_da1              : 64;
        ip_da2              : 64;
    }
}

