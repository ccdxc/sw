#include "ingress.h"
#include "INGRESS_p.h"


struct s3_tbl_k k;
struct s3_tbl_pvm_cq_handler_d d;
struct phv_ p;

params = {
    storage_tx_q_state_push_start = 0xa0a0;
};

k = {
    storage_kivec0_w_ndx = 0x2;
    storage_kivec0_dst_lif_sbit8_ebit10 = 0x6;
    storage_kivec0_dst_qaddr_sbit0_ebit31 = 0xa0008020;
};

d = {
    cspec = 0;
    rsvd0 = 0;
    sq_head = 0xA100;
    sq_id = 4;
    cid = 0x0001;
    phase = 0;
    status = 0;
    dst_qaddr = 0x2A0403020;
}; 

