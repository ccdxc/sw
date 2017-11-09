# Segment Configuration Spec
meta:
    id: SEGMENT_VXLAN_DOS_SRC_DST

label       : dos_remote
type        : tenant
fabencap    : vxlan
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
networks    :
    sgenable: True
endpoints   :
    sgenable: False
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 2
