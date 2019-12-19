# Configuration Template.
meta:
    id: TENANT_RECIRC

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_RECIRC
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_NATIVE_RECIRC
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: ref://store/objects/id=SEC_PROF_NOOP_FTE

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_RECIRC
            - ref://store/specs/id=SESSION_TCP_RECIRC
#            - ref://store/specs/id=SESSION_ICMP_RECIRC
        ipv6:
            - ref://store/specs/id=SESSION_UDP_RECIRC
            - ref://store/specs/id=SESSION_TCP_RECIRC
#            - ref://store/specs/id=SESSION_ICMPV6_RECIRC
        mac:
#            - ref://store/specs/id=SESSION_MAC_RECIRC

lif: ref://store/specs/id=LIF_ETH
