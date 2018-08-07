# Segment Configuration Spec
meta:
    id: SEGMENT_SPAN_VXLAN

type        : span
fabencap    : vxlan
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 1
    pvlan   : 0
    direct  : 0
    remote  : 3 # 1 for Local 1 for ERSPAN and 1 for RSPAN_VXLAN
