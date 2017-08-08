#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct nvme_be_ssd_info_k k;
struct nvme_be_ssd_info_wrr_d d;
struct phv_ p;

k = {
    nsq_ctx_c_ndx_lo = 10; // 144 = phv[2=32]
    nsq_ctx_c_ndx_med = 12; // 128 = phv[2=48]
    nsq_ctx_c_ndx_hi = 14; // 112 = phv[2=64]
    nsq_ctx_p_ndx_lo = 2; // 96 = phv[2=80]
    nsq_ctx_p_ndx_med = 4; // 80 = phv[2=96]
    nsq_ctx_p_ndx_hi = 6; // 64 = phv[2=112]
    nsq_ctx_base_addr = 0x92001000;
    nsq_ctx_num_entries = 6; // power of 2
    intrinsic_table_addr_raw = 0x20020050; // 0 = phv[5=128]
    nvme_be_got_cmd = 1; // 0 : phv[5:200]
};

d = {
    lo_weight = 8;
    med_weight = 16;
    hi_weight = 32;
    lo_running = 10;
    med_running = 9;
    hi_running = 8;
    num_running = 27;
    max_cmds = 64;
};


