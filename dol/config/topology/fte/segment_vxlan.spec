# Segment Configuration Spec
meta:
    id: SEGMENT_VXLAN_FTE

type        : tenant
native      : False
fabencap    : vxlan
broadcast   : flood
multicast   : flood
l4lb        : False
eplearn :
    arp_entry_timeout : 99999
    dhcp              : True
endpoints   :
    sgenable: True
    useg    : 0
    pvlan   : 4
    direct  : 0
    remote  : 8 # Remote TEPs
