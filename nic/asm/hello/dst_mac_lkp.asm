#include "ingress.h"
#include "INGRESS_p.h"
#include "sdk/platform/capri/capri_p4.h"
#include "p4/common/defines.h"

struct dst_mac_lkp_k    k;
struct dst_mac_lkp_d    d;
struct phv_             p;

%%

.align
dst_mac_lkp_miss:
    phvwr.c1        p.capri_intrinsic_tm_oport, TM_PORT_UPLINK_1
    phvwr.e         p.capri_intrinsic_drop, 1
    nop

.align
update_port:
    phvwr.e         p.capri_intrinsic_tm_oport, TM_PORT_UPLINK_1
    phvwr           p.capri_p4_intrinsic_valid, 1


.align
update_port_and_modify_vlan:
    seq             c1, k.ctag_1_valid, 1
    bcf             [c1], ctag_1_present
    phvwr           p.ctag_1_vid, d.u.update_port_and_modify_vlan_d.vid
    phvwr           p.ctag_1_valid, 1
    phvwr           p.ctag_1_etherType, k.ethernet_1_etherType
    phvwr           p.ethernet_1_etherType, ETHERTYPE_CTAG

ctag_1_present:
    phvwr.e         p.capri_intrinsic_tm_oport, TM_PORT_UPLINK_1
    phvwr           p.capri_p4_intrinsic_valid, 1
