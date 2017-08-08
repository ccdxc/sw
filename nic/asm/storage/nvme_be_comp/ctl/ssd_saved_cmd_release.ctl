#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct ssd_saved_cmds2_k k;
struct ssd_saved_cmds2_release_d d;
struct phv_ p;

k = {
    scq_ctx_idx = 7;
    nvme_be_cmd_index = 5;
    intrinsic_table_addr_raw = 0x20028c38;
};

d = {
    bitmap = 0xFFFFFFFFFFFFFFFF;
};

