# Configuration Template.
meta:
    id: TENANT_CACHEPERF

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_CACHEPERF
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_TCP_CACHEPERF
        ipv6:
            - ref://store/specs/id=SESSION_TCP_CACHEPERF
        mac:

lif: ref://store/specs/id=LIF_ETH
