# Configuration Template.
meta:
    id: TENANT_FP

type    : tenant
overlay : None

segments:
    - spec  : ref://store/specs/id=SEGMENT_REMOTE
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

sessions:
    unidest:
        scale: 1
        ipv4:
            - ref://store/specs/id=SESSION_UDP_BASIC
lif:
    spec        : ref://store/specs/id=LIF_ETH
    count       : 1
