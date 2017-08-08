#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct ssd_saved_cmds1_k k;
struct ssd_saved_cmds1_handle_d d;
struct phv_ p;


k = {
    scq_ctx_idx = 7; // 392 = phv[2=0]
    intrinsic_table_addr_raw = 0x2002cc40; // 8 = phv[5=128]
};

d = {
    src_queue_id = 11;
    ssd_handle = 2;
    io_priority = 1;
    is_read = 1;
    cmd_handle = 0x50015000;
};


