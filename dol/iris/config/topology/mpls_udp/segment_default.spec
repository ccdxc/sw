# Segment Configuration Spec
meta:
    id: MPLS_UDPTOPO_SEGMENT_DEFAULT

type        : tenant
fabencap    : vlan
native      : True
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    enics   : False
    classic : 4
    remote  : 1
