#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct nvme_be_ssd_info_k k;
struct nvme_be_ssd_info_update_weights_d d;
struct phv_ p;

k = {
    nvme_be_cmd_io_priority = 0;
    intrinsic_table_addr_raw = 0x20020038; // 8 = phv[5=128]
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
