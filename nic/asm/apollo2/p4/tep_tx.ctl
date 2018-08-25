#include "egress.h"
#include "EGRESS_p.h"
#include "apollo.h"

struct tep_tx_k     k;
struct tep_tx_d     d;
struct phv_         p;

k = {
    rewrite_metadata_dst_slot_id = 0x11111;
    p4e_apollo_i2e_src_slot_id_sbit0_ebit15 = 0x2222;
    p4e_apollo_i2e_src_slot_id_sbit16_ebit19 = 0x2;
    rewrite_metadata_mytep_ip = 0xAAAAAAAA;
    rewrite_metadata_encap_type = 0x0;
};

d = {
    tep_tx_d.dipo = 0xBBBBBBBB;
    tep_tx_d.dmac = 0x000101010101;
};

c1 = 0;
r5 = 0x000202020202;
