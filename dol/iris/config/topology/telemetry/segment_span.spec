# Segment Configuration Spec
meta:
    id: SEGMENT_SPAN_TELEMETRY

type        : span
fabencap    : vlan
native      : False
broadcast   : flood
multicast   : drop
l4lb        : False
endpoints   :
    sgenable: True
    useg    : 0
    pvlan   : 4
    direct  : 0
    remote  : 6
