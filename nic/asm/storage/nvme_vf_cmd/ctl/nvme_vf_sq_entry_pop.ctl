#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct nvme_vf_sq_entry_k k;
struct nvme_vf_sq_entry_qpop_d d;
struct phv_ p;

k = {
    nvme_vf_sq_ctx_idx = 0; // 283 = phv[1=0]
    nvme_vf_sq_ctx_state = 0; // 267 = phv[1=16]
    nvme_vf_sq_ctx_c_ndx = 0; // 251 = phv[1=32]
    nvme_vf_sq_ctx_num_entries = 6; // 187 = phv[1=256]
    nvme_vf_sq_ctx_sq_id = 4; // 171 = phv[1=336]
    nvme_vf_sq_ctx_paired_q_idx = 0; // 155 = phv[1=400]
    intrinsic_table_addr_raw = 0x97008000; // 299 = phv[0=149]
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
    dptr1 = 0xA0010100;
    dptr2 = 0xA0010200;
    slba = 0x20010000;
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

