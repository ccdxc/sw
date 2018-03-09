#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct tunnel_rewrite_k k;
struct tunnel_rewrite_d d;
struct phv_             p;

k = {
  ethernet_srcAddr = 0x001122334455;
  ethernet_dstAddr = 0x00aabbccddee;
  ethernet_etherType = 0x0800;
  rewrite_metadata_entropy_hash = 0xbaba;
  rewrite_metadata_tunnel_vnid = 0x164;
};

d = {
  u.encap_vxlan_d.mac_da = 0x00deadcafe00;
  u.encap_vxlan_d.mac_sa = 0x00deadcafe01;
  u.encap_vxlan_d.vlan_valid = 1;
  u.encap_vxlan_d.vlan_id = 0x65;
  u.encap_vxlan_d.ip_type = 0;
  u.encap_vxlan_d.ip_sa = 192.168.1.100;
  u.encap_vxlan_d.ip_da = 10.1.1.100;
};
