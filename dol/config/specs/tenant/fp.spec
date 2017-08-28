# Configuration Template.
meta:
    id: TENANT_FP

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_REMOTE
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: ref://store/objects/id=SEC_PROF_DEFAULT

sessions:
    unidest:
        scale: 1
        ipv4:
            - ref://store/specs/id=SESSION_UDP_FP
            - ref://store/specs/id=SESSION_UDP_SERVICES
        ipv6: None
        mac: None

lif: ref://store/specs/id=LIF_ETH
