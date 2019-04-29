# Segment Configuration Spec
meta:
    id: SEGMENT_NVME_PROXY

type        : tenant
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    sgenable: False
    useg    : 0
    pvlan   : 1
    direct  : 0
    remote  : 2 # 1 EP per uplink
