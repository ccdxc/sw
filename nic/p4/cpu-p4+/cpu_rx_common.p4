/***************************************************
 * cpu_rx_common.p4
 **************************************************/
header_type cpu_common_to_stage_phv_t {
    fields {
        page                    : 32;
        descr                   : 32;
	    arqrx_base              : 32;
        arqrx_pindex            : 16;
        payload_len             : 16;
    }
}

header_type arq_rx_pi_d_t {
    fields {
        pi_0                    : 16;
        pi_1                    : 16;
        pi_2                    : 16;
    }    
}
#define ARQ_RX_PI_PARAMS                            \
pi_0, pi_1, pi_2
#

#define GENERATE_ARQ_RX_PI_D                        \
    modify_field(arq_rx_pi_d.pi_0, pi_0);           \
    modify_field(arq_rx_pi_d.pi_1, pi_1);           \
    modify_field(arq_rx_pi_d.pi_2, pi_2);


header_type arq_tx_pi_d_t {
    fields {
        pi_0                    : 16;
        pi_1                    : 16;
        pi_2                    : 16;
    }    
}


#define ARQ_TX_PI_PARAMS                            \
pi_0, pi_1, pi_2
#

#define GENERATE_ARQ_TX_PI_D                        \
    modify_field(arq_tx_pi_d.pi_0, pi_0);           \
    modify_field(arq_tx_pi_d.pi_1, pi_1);           \
    modify_field(arq_tx_pi_d.pi_2, pi_2);
