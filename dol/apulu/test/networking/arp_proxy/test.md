# Testcases for ARP Proxy functionality

Initial setup:
    a) Configuration:
            On the DUT VPCs, subnets, VNICs, local mappings, remote mappings,
            route tables, tunnels, nexthops, security groups and policies are
            configured.

## ARP request for known remote IP
    Trigger:
        ARP request for a destination from any remote mapping is sent to host
        interface.

    Expectation:
        a) ARP reply with destination mac from the remote mapping is sent back
           on host interface.

## ARP request for unknown IP
    Trigger:
        ARP request for an unknown IP is sent to host interface.

    Expectation:
        a) ARP reply with VR MAC from originating subnet used as destination mac
           is sent back on host interface.
