
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_stage0_packet_len_k.h"

#include "nic/p4/common/defines.h"

struct rx_stage0_packet_len_k_ k;
struct phv_ p;

%%

.param      eth_rx_packet_len
.param      cpu_rx_packet_len

.align
rx_stage0_packet_len:

sub         r1, k.p4_intr_frame_size, k.p4_rxdma_intr_rx_splitter_offset
seq         c1, k.app_header_app_type, P4PLUS_APPTYPE_CLASSIC_NIC
j.c1        eth_rx_packet_len
seq         c2, k.app_header_app_type, P4PLUS_APPTYPE_CPU
j.c2        cpu_rx_packet_len

nop.e
nop
