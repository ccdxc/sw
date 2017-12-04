# Configuration Template.
meta:
    id: TENANT_TELEMETRY_VXLAN

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_VXLAN_TELEMETRY
      count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_TCP_TELEMETRY
        ipv6: None
        mac: None

lif: ref://store/specs/id=LIF_ETH
