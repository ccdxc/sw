#include "nw.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_vf_properties_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct vf_properties_k_ k;
struct vf_properties_d d;
struct phv_   p;

%%

nop:
    nop.e
    nop

.align
vf_properties:
    bbne            k.control_metadata_uplink, TRUE, vf_properties_host
vf_properties_uplink:
    seq             c1, k.mpls_0_label, d.u.vf_properties_d.mpls_in1
    seq.!c1         c1, k.mpls_0_label, d.u.vf_properties_d.mpls_in2
    seq             c2, k.inner_ipv4_dstAddr, d.u.vf_properties_d.overlay_ip1
    seq.!c2         c2, k.inner_ipv4_dstAddr, d.u.vf_properties_d.overlay_ip2
    setcf           c1, [c1 & c2]
    phvwr.!c1.e     p.control_metadata_drop_reason[DROP_VF_IP_LABEL_MISMATCH], 1
    phvwr.!c1       p.capri_intrinsic_drop, TRUE
    seq             c1, k.control_metadata_src_class_e, TRUE
    nop.!c1.e
    seq             c1, k.control_metadata_record_route_inner_dst_ip, r0
    seq.!c1         c1, k.control_metadata_record_route_inner_dst_ip[31:28], 0xE
    add             r1, r0, -1
    sub             r2, 32, d.u.vf_properties_d.gw_prefix_len
    sll             r1, r1, r2
    and             r2, k.control_metadata_record_route_inner_dst_ip, r1
    sne.!c1         c1, r2[31:0], d.u.vf_properties_d.gw_prefix
    nop.!c1.e
    phvwr.c1.e      p.control_metadata_drop_reason[DROP_VF_BAD_RR_DST_IP], 1
    phvwr           p.capri_intrinsic_drop, TRUE

vf_properties_host:
    phvwr           p.tunnel_metadata_tunnel_originate, \
                        d.u.vf_properties_d.tunnel_originate
    phvwr           p.rewrite_metadata_tunnel_vnid, d.u.vf_properties_d.mpls_out
    phvwr           p.rewrite_metadata_tunnel_rewrite_index, \
                        d.u.vf_properties_d.tunnel_rewrite_index
    seq             c1, k.control_metadata_dst_class_e, TRUE
    nop.!c1.e
    seq             c1, k.control_metadata_record_route_dst_ip, r0
    seq.!c1         c1, k.control_metadata_record_route_dst_ip[31:28], 0xE
    add             r1, r0, -1
    sub             r2, 32, d.u.vf_properties_d.gw_prefix_len
    sll             r1, r1, r2
    and             r2, k.control_metadata_record_route_dst_ip, r1
    sne.!c1         c1, r2[31:0], d.u.vf_properties_d.gw_prefix
    phvwr.!c1.e     p.nat_metadata_nat_ip[127:96], \
                        k.control_metadata_record_route_dst_ip
    phvwr.c1.e      p.control_metadata_drop_reason[DROP_VF_BAD_RR_DST_IP], 1
    phvwr           p.capri_intrinsic_drop, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vf_properties_error:
    nop.e
    nop
