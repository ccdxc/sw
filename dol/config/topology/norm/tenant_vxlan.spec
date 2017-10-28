# Configuration Template.
meta:
    id: NORM_TENANT_OVERLAY_VXLAN

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=NORM_SEGMENT_VXLAN
      count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 4

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_TCP_NORM
            - ref://store/specs/id=SESSION_UDP_NORM
            - ref://store/specs/id=SESSION_ICMP_NORM
        ipv6:
            - ref://store/specs/id=SESSION_TCP_NORM
            - ref://store/specs/id=SESSION_UDP_NORM
            - ref://store/specs/id=SESSION_ICMPV6_NORM
        mac:

lif: ref://store/specs/id=LIF_ETH
