# Configuration Template.
meta:
    id: TENANT_FTE

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_FTE
      count : 2
    - spec  : ref://store/specs/id=SEGMENT_NATIVE_FTE
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: ref://store/objects/id=SEC_PROF_NOOP_FTE

fte:
    ep_learn:
        ipv4: True
        ipv6: True

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
