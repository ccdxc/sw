# Configuration Template.
meta:
    id: TENANT_FTE

type    : tenant
overlay : vlan
label   : DOS

segments:
    - spec  : ref://store/specs/id=SEGMENT_FTE
      count : 1
    #- spec  : ref://store/specs/id=SEGMENT_REMOTE_FTE
    #  count : 1
    #- spec  : ref://store/specs/id=SEGMENT_REMOTEV6_FTE
    #  count : 1
    #- spec  : ref://store/specs/id=SEGMENT_NATIVE_FTE
    #  count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

dos: ref://store/specs/id=DOS

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_FTE
            - ref://store/specs/id=SESSION_TCP_FTE
            - ref://store/specs/id=SESSION_ICMP_FTE
            - ref://store/specs/id=SESSION_PR255_DOS
        ipv6:
            - ref://store/specs/id=SESSION_UDP_FTE
            - ref://store/specs/id=SESSION_TCP_FTE
            - ref://store/specs/id=SESSION_ICMPV6_FTE
            - ref://store/specs/id=SESSION_PR255_DOS
        mac:
            - ref://store/specs/id=SESSION_MAC_FTE

lif: ref://store/specs/id=LIF_ETH
