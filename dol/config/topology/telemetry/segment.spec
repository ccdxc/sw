# Segment Configuration Spec
meta:
    id: SEGMENT_TELEMETRY

type        : tenant
native      : False
broadcast   : flood
multicast   : drop
l4lb        : False
endpoints   :
    useg    : 2
    pvlan   : 0
    direct  : 0
    remote  : 4 # Remote TEPs
