# Segment Configuration Spec
meta:
    id: SEGMENT_EPLEARN

type        : tenant
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
proxy_arp_enabled : True
eplearn :
    remote            : False
    arp_entry_timeout : 10
    arp_probe         : True
    dhcp              : True
endpoints   :
    sgenable: True
    useg    : 0
    pvlan   : 4
    direct  : 0
    remote  : 2
