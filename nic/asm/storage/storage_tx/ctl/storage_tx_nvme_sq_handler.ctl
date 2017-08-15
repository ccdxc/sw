#include "ingress.h"
#include "INGRESS_p.h"


struct s1_tbl_k k;
struct s1_tbl_nvme_sq_entry_pop_d d;
struct phv_ p;

params = {
    storage_tx_q_state_push_start = 0xa0a0;
};

k = {
    storage_kivec0_w_ndx = 0x1;
    storage_kivec0_dst_lif_sbit8_ebit10 = 0x5;
    storage_kivec0_dst_qaddr_sbit0_ebit31 = 0xa0008000;
};

d = {
    opc = 1;
    fuse = 0;
    rsvd0 = 0;
    psdt = 0;
    cid = 0x1234;
    nsid = 0xababcdcd;
    rsvd2 = 0;
    rsvd3 = 0;
    mptr = 0;
    dptr1 = 0x80000000A0010100;
    dptr2 = 0x80000000A0010200;
    slba = 0x8000000020010000;
    nlb = 1;
    rsvd12 = 0;
    prinfo = 0;
    fua = 0;
    lr = 0;
    dsm = 0;
    rsvd13 = 0;
    dw14 = 0;
    dw15 = 0;
}; 

