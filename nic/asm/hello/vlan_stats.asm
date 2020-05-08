#include "ingress.h"
#include "INGRESS_p.h"
#include "sdk/platform/capri/capri_p4.h"

struct vlan_stats_k k;
struct vlan_stats_d d;
struct phv_         p;

%%

update_stats:
    tbladd          d.update_stats_d.in_pkts, 1
    sub.e           r1, k.{capri_p4_intrinsic_frame_size_sbit0_ebit5,\
                        capri_p4_intrinsic_frame_size_sbit6_ebit13}, \
                        ASICPD_GLOBAL_INTRINSIC_HDR_SZ
    tbladd          d.update_stats_d.in_bytes, r1
