# Configuration Template.
meta:
    id: TENANT_MULTICAST_VXLAN

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_MULTICAST_VXLAN
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=MULTICAST_SESSION_UDP
        ipv6:
            - ref://store/specs/id=MULTICAST_SESSION_UDP
            - ref://store/specs/id=MULTICAST_SESSION_ICMPV6
        mac: None

lif: ref://store/specs/id=LIF_ETH
