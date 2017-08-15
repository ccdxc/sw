# Segment Configuration Spec
meta:
    id: SEGMENT_SPAN

type        : span
native      : False
broadcast   : drop
multicast   : drop
endpoints   :
    useg    : 1
    pvlan   : 0
    direct  : 0
    remote  : 2 # 1 for ERSPAN and 1 for RSPAN_VXLAN
