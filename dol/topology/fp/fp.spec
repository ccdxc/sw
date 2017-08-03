# Configuration Spec
uplink:
    - entry:
        id      : Uplink1
        port    : 1
        mode    : TRUNK
        sriov   : True
        status  : UP

uplinkpc: None
tenants:
    -   spec    : ref://store/specs/id=TENANT_FP
        count   : 1
        lifns   : range/101/128
