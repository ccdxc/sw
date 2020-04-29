#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_input_properties_mac_vlan_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct input_properties_mac_vlan_k_ k;
struct input_properties_mac_vlan_d  d;
struct phv_                         p;

%%

input_properties_mac_vlan:
    bbne            k.p4plus_to_p4_valid, TRUE, input_properties_mac_vlan_tcp
    phvwr           p.control_metadata_src_lif, k.capri_intrinsic_lif

    bbne            k.p4plus_to_p4_update_ip_len, TRUE, input_properties_mac_vlan_tcp
    phvwr           p.flow_lkp_metadata_lkp_inst, k.p4plus_to_p4_lkp_inst
    sub             r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l4_2
    sub             r1, r1, k.tcp_dataOffset, 2
    b               input_properties_mac_vlan_common
    phvwr           p.l4_metadata_tcp_data_len, r1

input_properties_mac_vlan_tcp:
    bbne            k.tcp_valid, TRUE, input_properties_mac_vlan_common
    crestore        [c5-c2], k.{inner_ipv4_valid,inner_ipv6_valid, \
                        ipv4_valid,ipv6_valid}, 0xF
    .brbegin
    bcpri           [c5-c2], [0,1,2,3]
    nop
    .brcase 0
    // c2
    sub             r7, k.ipv6_payloadLen, k.tcp_dataOffset, 2
    b               input_properties_mac_vlan_common
    phvwr           p.l4_metadata_tcp_data_len, r7
    .brcase 1
    // c3
    add             r6, k.ipv4_ihl, k.tcp_dataOffset
    sub             r7, k.ipv4_totalLen, r6, 2
    b               input_properties_mac_vlan_common
    phvwr           p.l4_metadata_tcp_data_len, r7
    .brcase 2
    // c4
    sub             r6, k.inner_ipv6_payloadLen, k.tcp_dataOffset, 2
    b               input_properties_mac_vlan_common
    phvwr           p.l4_metadata_tcp_data_len, r6
    .brcase 3
    // c5
    add             r6, k.inner_ipv4_ihl, k.tcp_dataOffset
    sub             r7, k.inner_ipv4_totalLen, r6, 2
    b               input_properties_mac_vlan_common
    phvwr           p.l4_metadata_tcp_data_len, r7
    .brcase 4
    // none
    b               input_properties_mac_vlan_common
    nop
    .brend

input_properties_mac_vlan_common:
    seq             c2, k.recirc_header_valid, TRUE
    phvwr.c2        p.control_metadata_recirc_reason, k.recirc_header_reason[1:0]
    phvwr.c2        p.qos_metadata_qos_class_id[4:0], k.capri_intrinsic_tm_iq[4:0]

    sub             r1, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l2_1
    seq             c2, k.p4plus_to_p4_insert_vlan_tag, TRUE
    add.c2          r1, r1, 4

    // if table lookup is miss, return
    nop.!c1.e
    phvwr           p.capri_p4_intrinsic_packet_len, r1

    // clear mirror, return
	seq             c1, d.input_properties_mac_vlan_d.clear_ingresss_mirror, TRUE
	phvwr.c1.e      p.control_metadata_clear_ingresss_mirror, \
	                    d.input_properties_mac_vlan_d.clear_ingresss_mirror
    // skip_flow_update, return
    seq             c2, d.input_properties_mac_vlan_d.skip_flow_update, TRUE
    phvwr.c2.e      p.control_metadata_skip_flow_update, \
                        d.input_properties_mac_vlan_d.skip_flow_update
    phvwr.!c2       p.flow_miss_metadata_tunnel_originate, \
                        d.input_properties_mac_vlan_d.tunnel_originate
    phvwr           p.control_metadata_ep_learn_en, \
                        d.input_properties_mac_vlan_d.ep_learn_en
    phvwr           p.control_metadata_clear_ingresss_mirror, \
                        d.input_properties_mac_vlan_d.clear_ingresss_mirror

    // input_properties call
    or              r1, d.input_properties_mac_vlan_d.src_lport, \
                        d.input_properties_mac_vlan_d.dst_lport, 16
    phvwrpair       p.{control_metadata_dst_lport,control_metadata_src_lport}, \
                        r1, p.flow_lkp_metadata_lkp_vrf, \
                        d.input_properties_mac_vlan_d.vrf

    or              r2, d.input_properties_mac_vlan_d.mirror_on_drop_session_id, \
                        d.input_properties_mac_vlan_d.mirror_on_drop_en, 8
    phvwr           p.{control_metadata_mirror_on_drop_en, \
                        control_metadata_mirror_on_drop_session_id}, r2

    phvwr           p.capri_intrinsic_tm_replicate_ptr, \
                        d.input_properties_mac_vlan_d.flow_miss_idx
    phvwr           p.control_metadata_flow_miss_qos_class_id, \
                        d.input_properties_mac_vlan_d.flow_miss_qos_class_id

    or              r1, d.input_properties_mac_vlan_d.vrf, \
                        d.input_properties_mac_vlan_d.ipsg_enable, 17
    phvwr           p.{control_metadata_ipsg_enable, \
                        control_metadata_clear_promiscuous_repl, \
                        flow_lkp_metadata_lkp_reg_mac_vrf}, r1

    or              r1, 0x13, \
                        d.input_properties_mac_vlan_d.mdest_flow_miss_action, 5
    phvwrm          p.{control_metadata_mdest_flow_miss_action, \
                        control_metadata_if_label_check_en, \
                        control_metadata_if_label_check_fail_drop,\
                        control_metadata_skip_flow_update, \
                        control_metadata_mseg_bm_bc_repls, \
                        control_metadata_mseg_bm_mc_repls}, r1, 0x7B

    or              r1, d.input_properties_mac_vlan_d.tunnel_vnid, \
                        d.input_properties_mac_vlan_d.tunnel_rewrite_index, 24
    or              r1, r1, d.input_properties_mac_vlan_d.rewrite_index, 40
    phvwr           p.{rewrite_metadata_rewrite_index, \
                        rewrite_metadata_tunnel_rewrite_index, \
                        rewrite_metadata_tunnel_vnid},  r1

    phvwr           p.flow_lkp_metadata_lkp_dir, \
                        d.input_properties_mac_vlan_d.dir
    phvwr           p.l4_metadata_profile_idx, \
                        d.input_properties_mac_vlan_d.l4_profile_idx
    or.e            r1, 0x11, d.input_properties_mac_vlan_d.src_if_label, 1
    phvwr.f         p.{control_metadata_flow_learn, \
                        control_metadata_src_if_label, \
                        control_metadata_uuc_fl_pe_sup_en}, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
input_properties_mac_vlan_error:
  nop.e
  nop
