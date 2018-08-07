# Configuration Template.
meta:
    id: TENANT_STORAGE

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_STORAGE
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        scale: 1
        ipv4:
            - ref://store/specs/id=SESSION_STORAGE
        ipv6:
        mac:

lif: ref://store/specs/id=LIF_STORAGE
