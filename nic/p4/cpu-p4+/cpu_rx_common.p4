/***************************************************
 * cpu_rx_common.p4
 **************************************************/
#ifdef DO_NOT_USE_CPU_SEM

header_type arq_pi_d_t {
    fields {
        pi_0                    : 16;
        pi_1                    : 16;
        pi_2                    : 16;
    }    
}

#define ARQ_PI_PARAMS       \
pi_0, pi_1, pi_2

#define GENERATE_ARQ_PI_D(_d_struct) \
    modify_field(_d_struct.pi_0, pi_0);           \
    modify_field(_d_struct.pi_1, pi_1);           \
    modify_field(_d_struct.pi_2, pi_2);
   
#else

header_type arq_pi_d_t {
    fields {
        arq_pindex              : 32;
    }    
}

#define ARQ_PI_PARAMS       \
arq_pindex

#define GENERATE_ARQ_PI_D(_d_struct)  \
    modify_field(_d_struct.arq_pindex, arq_pindex);

#endif
