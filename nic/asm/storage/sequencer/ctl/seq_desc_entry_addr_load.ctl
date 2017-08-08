#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct seq_desc_entry_addr_k k;
struct seq_desc_entry_addr_load_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0xC0002000; // 0 = phv[5=128]
};

d = {
    desc_entry_addr = 0xC0004000;
};

