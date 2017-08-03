# Configuration Template.
meta:
    id: TENANT_ALL_ENICS

type    : tenant
overlay : None

segments:
    - spec  : ref://store/specs/id=SEGMENT_MDST_FWD
      count : 2

    #- spec  : ref://store/specs/id=SEGMENT_MDST_DROP
    #  count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 4

security_profile: ref://store/objects/id=SEC_PROF_DEFAULT

sessions:
    unidest:
        scale: 16
        ipv4:
            - ref://store/specs/id=SESSION_TCP_BASIC
            - ref://store/specs/id=SESSION_UDP_BASIC
            - ref://store/specs/id=SESSION_ICMP_BASIC
            - ref://store/specs/id=SESSION_TCP_PROXY_BASIC

        ipv6:
            - ref://store/specs/id=SESSION_TCP_BASIC
            - ref://store/specs/id=SESSION_UDP_BASIC
            - ref://store/specs/id=SESSION_ICMPV6_BASIC
            - ref://store/specs/id=SESSION_TCP_PROXY_BASIC
        l2:
            - ref://store/specs/id=SESSION_MAC_BASIC

    multidest:
        scale: 16
        ipv4:
            - ref://store/specs/id=SESSION_L2MC_BASIC
            - ref://store/specs/id=SESSION_L2BC_BASIC
        ipv6:
            - ref://store/specs/id=SESSION_L2MC_BASIC
            - ref://store/specs/id=SESSION_L2BC_BASIC
        l2:
            - ref://store/specs/id=SESSION_L2MC_BASIC
            - ref://store/specs/id=SESSION_L2BC_BASIC

lif:
    spec        : ref://store/specs/id=LIF_ETH
    count       : 128
