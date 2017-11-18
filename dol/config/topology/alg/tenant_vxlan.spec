# Configuration Template.
meta:
    id: TENANT_VXLAN_FTE

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_VXLAN_FTE
      count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_FTE
            - ref://store/specs/id=SESSION_TCP_FTE
            - ref://store/specs/id=SESSION_ICMP_FTE
        ipv6:
            - ref://store/specs/id=SESSION_UDP_FTE
            - ref://store/specs/id=SESSION_TCP_FTE
            - ref://store/specs/id=SESSION_ICMPV6_FTE
        mac:
            - ref://store/specs/id=SESSION_MAC_FTE

lif: ref://store/specs/id=LIF_ETH
