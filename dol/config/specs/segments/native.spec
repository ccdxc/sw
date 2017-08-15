# Segment Configuration Spec
meta:
    id: SEGMENT_NATIVE

type        : tenant
native      : True
broadcast   : flood
multicast   : replicate
endpoints   :
    useg    : 2
    pvlan   : 2
    direct  : 0
    remote  : 4 # 1 EP per uplink
