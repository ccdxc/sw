#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct ssd_cmds_k k;
struct ssd_cmds_save_d d;
struct phv_ p;

k = {
    nsq_ctx_idx = 10; // 392 : phv[2:0]
    nvme_be_cmd_nvme_cmd_lo = 0; // 136 : phv[4:0]
    intrinsic_table_addr_raw = 0x2002f6d0; // 8 : phv[5:128]
};

d = {
    bitmap = 0xD;
};


