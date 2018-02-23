# Configuration Template.
meta:
    id: TENANT_GFT_VXLAN

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_GFT_VXLAN
      count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

gft_sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_GFT
            - ref://store/specs/id=SESSION_TCP_GFT
            - ref://store/specs/id=SESSION_ICMP_GFT
        ipv6:
            - ref://store/specs/id=SESSION_UDP_GFT
            - ref://store/specs/id=SESSION_TCP_GFT
            - ref://store/specs/id=SESSION_ICMPV6_GFT
        mac:
            - ref://store/specs/id=SESSION_MAC_GFT

lif: ref://store/specs/id=LIF_ETH
