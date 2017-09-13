# Configuration Template.
meta:
    id: TENANT_ETH_VXLAN

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_ETH_VXLAN
      count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_ETH_VXLAN
            - ref://store/specs/id=SESSION_TCP_ETH_VXLAN
            - ref://store/specs/id=SESSION_ICMP_ETH_VXLAN
        ipv6:
            - ref://store/specs/id=SESSION_UDP_ETH_VXLAN
            - ref://store/specs/id=SESSION_TCP_ETH_VXLAN
            - ref://store/specs/id=SESSION_ICMPV6_ETH_VXLAN
        mac:
            - ref://store/specs/id=SESSION_MAC_ETH_VXLAN

lif: ref://store/specs/id=LIF_ETH
