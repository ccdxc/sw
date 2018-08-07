# Configuration Template.
meta:
    id: VXLANTOPO_TENANT_OVERLAY_VXLAN

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=VXLANTOPO_SEGMENT_DEFAULT
      count : 1
    - spec  : ref://store/specs/id=VXLANTOPO_SEGMENT_VXLAN
      count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 4

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=VXLANTOPO_SESSION_TCP
            - ref://store/specs/id=VXLANTOPO_SESSION_UDP
            - ref://store/specs/id=VXLANTOPO_SESSION_ICMP
        ipv6:
            - ref://store/specs/id=VXLANTOPO_SESSION_TCP
            - ref://store/specs/id=VXLANTOPO_SESSION_UDP
            - ref://store/specs/id=VXLANTOPO_SESSION_ICMPV6
        mac:
            - ref://store/specs/id=VXLANTOPO_SESSION_MAC

lif: ref://store/specs/id=LIF_ETH
