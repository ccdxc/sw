# Segment Configuration Spec
meta:
    id: SEGMENT_INFRA_TELEMETRY

type        : infra
native      : False
fabencap    : vlan
broadcast   : drop
multicast   : drop
l4lb        : False
endpoints   :
    sgenable: True
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 4 # Remote TEPs
