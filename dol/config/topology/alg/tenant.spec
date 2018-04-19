# Configuration Template.
meta:
    id: TENANT_ALG

type    : tenant
overlay : vlan

segments:
    #- spec  : ref://store/specs/id=SEGMENT_ALG
    #  count : 2
    - spec  : ref://store/specs/id=SEGMENT_NATIVE_ALG
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: ref://store/objects/id=SEC_PROF_ALG

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_ALG
            - ref://store/specs/id=SESSION_TCP_ALG
        ipv6:
            - ref://store/specs/id=SESSION_UDP_ALG
            - ref://store/specs/id=SESSION_TCP_ALG
        mac:

lif: ref://store/specs/id=LIF_ETH
