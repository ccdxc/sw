/******************************************************************************/
TABLE : ipsg
/******************************************************************************/
key format : 1
phv.ipsg_enable == TRUE
ipv4.valid == TRUE
struct k {
  phv.lkp_vrf : 16;
  phv.ethernet_srcAddr : 48;
  phv.ipv4_srcAddr : 32;
}

key format : 2
phv.ipsg_enable == TRUE
ipv6.valid == TRUE
struct k {
  phv.lkp_vrf : 16;
  phv.ethernet_srcAddr : 48;
  phv.ipv6_srcAddr : 32;
}

ACTION ipsg.ipsg_deny
struct d {
}
phvwr.e phv.control_drop_drop, TRUE
nop
