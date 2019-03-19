#include "egress.h"
#include "EGRESS_p.h"


struct egress_local_vnic_info_rx_k k;
struct egress_local_vnic_info_rx_d d;
struct phv_                     p;

d = {
    egress_local_vnic_info_rx_d.overlay_mac = 0xAABBCCDDEEFF;
    egress_local_vnic_info_rx_d.overlay_vlan_id = 0x123;
};
c1 = 0;
