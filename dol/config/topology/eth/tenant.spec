# Configuration Template.
meta:
    id: TENANT_ETH

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_ETH
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_BLACKHOLE
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_NATIVE_ETH
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_ETH
            - ref://store/specs/id=SESSION_TCP_ETH
            - ref://store/specs/id=SESSION_ICMP_ETH
        ipv6:
            - ref://store/specs/id=SESSION_UDP_ETH
            - ref://store/specs/id=SESSION_TCP_ETH
            - ref://store/specs/id=SESSION_ICMPV6_ETH
        mac:
            - ref://store/specs/id=SESSION_MAC_ETH

lif: ref://store/specs/id=LIF_ETH
