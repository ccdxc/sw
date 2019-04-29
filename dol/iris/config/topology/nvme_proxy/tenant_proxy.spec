# Configuration Template.
meta:
    id: TENANT_NVME_PROXY

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_NVME_PROXY
      count : 1

nfveps: 0

security_profile: None
security_policy: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_NVME_PROXY

lif: ref://store/specs/id=LIF_NVME
