# Configuration Template.
meta:
    id: TENANT_FIREWALL

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_FIREWALL
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: ref://store/objects/id=SEC_PROF_FIREWALL

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_TCP_FIREWALL
        ipv6:
            - ref://store/specs/id=SESSION_TCP_FIREWALL
        mac:

lif: ref://store/specs/id=LIF_ETH
