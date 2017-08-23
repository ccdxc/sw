#include "egress.h"
#include "EGRESS_p.h"

struct rewrite_k k;
struct rewrite_d d;
struct phv_      p;

d = {
  u.rewrite_d.mac_sa = 0x001122334455;
  u.rewrite_d.mac_sa = 0x00aabbccddee;
};

k = {
  vlan_tag_valid = 1;
  vlan_tag_etherType = 0x800;
  rewrite_metadata_mac_sa_rewrite = 0;
  rewrite_metadata_mac_da_rewrite = 0;
  nat_metadata_nat_ip = 0xac640101;
  nat_metadata_nat_l4_port = 0xbaba;
  rewrite_metadata_ttl_dec = 1;
  qos_metadata_dscp_en = 0;
  ipv4_valid = 1;
  ipv6_valid = 0;
  ipv4_ttl = 64;
  ipv6_hopLimit = 128;
};
