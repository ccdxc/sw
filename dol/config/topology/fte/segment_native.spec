# Segment Configuration Spec
meta:
    id  : SEGMENT_NATIVE_FTE

type        : tenant
fabencap    : vlan
native      : True
broadcast   : flood
multicast   : flood
l4lb        : False
eplearn :
    remote            : False
    arp_entry_timeout : 99999
    dhcp              : True
endpoints   :
    sgenable: True
    useg    : 0
    pvlan   : 8
    direct  : 0
    remote  : 12
