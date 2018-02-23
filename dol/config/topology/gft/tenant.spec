# Configuration Template.
meta:
    id: TENANT_GFT

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_GFT
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_NATIVE_GFT
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_BLACKHOLE
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE

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
